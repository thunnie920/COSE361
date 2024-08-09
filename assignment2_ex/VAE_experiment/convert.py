import os, sys
import model
import argparse
import soundfile
import torch
import numpy as np
import pickle

from speech_tools import world_decode_mc, world_speech_synthesis

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
#device = torch.device('cpu') # train use cpu


def load_pickle(path):
    with open(path, 'rb') as f:
        return pickle.load(f)

def make_one_hot_vector(spk_idx, spk_num):
    vec = np.zeros(spk_num)
    vec[spk_idx] = 1.0
    return vec

def reparameterize(mu, logvar):
    std = torch.exp(0.5*logvar)
    eps = torch.randn_like(std)
    return eps.mul(std).add(mu)

spk_list = ["VCC2SF1", "VCC2SF2", "VCC2SM1", "VCC2SM2"]
SPK_DICT = {
    spk_idx:spk_id 
    for spk_idx, spk_id in enumerate(spk_list)
}
VEC_DICT = {
    spk_id:[make_one_hot_vector(spk_idx, len(spk_list))]
    for spk_idx, spk_id in SPK_DICT.items()
}

parser = argparse.ArgumentParser()
parser.add_argument('--model_type', default='')
parser.add_argument('--model_path', default='')
parser.add_argument('--convert_path', default='')
parser.add_argument('--epoch', type=int, default=0)
args = parser.parse_args()

model_dir = args.model_path
if args.convert_path=='':
    convert_path = "result/"+args.model_path.split("/")[-1]
else:
    convert_path = args.convert_path
latent_dim=8

Enc = model.Encoder(style_dim=4, latent_dim=latent_dim, vae_type=args.model_type)
if args.epoch == 0:
    Enc.load_state_dict(torch.load(model_dir+"/final_enc.pt"))
else:
    Enc.load_state_dict(torch.load(model_dir+"/parm/"+str(args.epoch)+"_enc.pt"))
Enc = Enc.to(device)
Enc.eval()
if args.model_type == "MD":
    Dec_dict=dict()
    for spk_id in spk_list:
        cur_Dec = model.Decoder(style_dim=4, latent_dim=latent_dim, vae_type=args.model_type)
        cur_Dec = cur_Dec.to(device)
        cur_Dec.eval()
        if args.epoch == 0:
            cur_Dec.load_state_dict(torch.load(model_dir+"/final_"+spk_id+"_dec.pt"))
        else:
            cur_Dec.load_state_dict(torch.load(model_dir+"/parm/"+str(args.epoch)+"_"+spk_id+"_dec.pt"))
        Dec_dict[spk_id]=cur_Dec
else:
    Dec = model.Decoder(style_dim=4, latent_dim=latent_dim, vae_type=args.model_type)
    if args.epoch == 0:
        Dec.load_state_dict(torch.load(model_dir+"/final_dec.pt"))
    else:
        Dec.load_state_dict(torch.load(model_dir+"/parm/"+str(args.epoch)+"_dec.pt"))
    Dec = Dec.to(device)
    Dec.eval()

feat_dir = "data/test"

sampling_rate = 22050
num_mcep = 36
frame_period = 5.0
n_frames = 128



STAT_DICT = dict()
for source_spk in spk_list:
    stat_path = "data/train/"+source_spk+"/feats.p"
    _, sp_m, sp_s, logf0_m, logf0_s = load_pickle(stat_path)
    STAT_DICT[source_spk] = (sp_m, sp_s, logf0_m, logf0_s)
    for target_spk in spk_list:
        os.makedirs(os.path.join(convert_path, source_spk+"_to_"+target_spk), exist_ok=True)

for source_spk in spk_list:
    print("Processing", source_spk)
    feat_path = os.path.join(feat_dir,source_spk)    
    sp_m_s, sp_s_s, logf0_m_s, logf0_s_s = STAT_DICT[source_spk]

    for _, _, file_list in os.walk(feat_path):
        for file_id in file_list:
            utt_id = file_id.split(".")[0]
            if utt_id == "ppg36":
                continue
            print("\tConvert {}.wav ...".format(utt_id))
            file_path = os.path.join(feat_path, file_id)
            x, f0, ap = load_pickle(file_path)

            # x = (coded_sp-sp_m_s) / sp_s_s
            x = np.expand_dims(x, axis=0)
            x = np.expand_dims(x, axis=0)
            x = torch.Tensor(x).float().to(device).contiguous()

            logf0_norm = (np.log(f0)-logf0_m_s) / logf0_s_s

            for target_spk in spk_list:
                style = VEC_DICT[target_spk]
                y = torch.Tensor(style).float().to(device).contiguous()

                with torch.no_grad():
                    z_mu, z_logvar, z = Enc(x, y)
                    if args.model_type == "MD":
                        new_sp, _, _ = Dec_dict[target_spk](z, y)
                    else:
                        new_sp, _, _ = Dec(z, y)

                new_sp = new_sp.double().cpu().numpy()[0][0]
                
                sp_m_t, sp_s_t, logf0_m_t, logf0_s_t = STAT_DICT[target_spk]

                new_f0 = np.exp(logf0_norm * logf0_s_t + logf0_m_t)
                new_sp = new_sp * sp_s_t + sp_m_t
                new_sp = np.ascontiguousarray(new_sp.T)
                new_sp = world_decode_mc(new_sp, fs=sampling_rate)
                
                wav_transformed = world_speech_synthesis(f0=new_f0, decoded_sp=new_sp, 
                    ap=ap, fs=sampling_rate, frame_period=frame_period)
                wav_transformed = np.nan_to_num(wav_transformed)
                soundfile.write(os.path.join(convert_path, source_spk+"_to_"+target_spk, utt_id+".wav"), wav_transformed, sampling_rate)
                
            
            # print(coded_sp.shape)
            # print(f0.shape)
            # print(ap.shape)
