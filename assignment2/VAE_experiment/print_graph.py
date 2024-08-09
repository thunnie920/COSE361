import argparse
parser = argparse.ArgumentParser()
parser.add_argument('--log_path', type=str)
parser.add_argument('--model_path', type=str)
parser.add_argument('--out_path', type=str)
args = parser.parse_args()

log_path = args.log_path
model_path = args.model_path
out_path = args.out_path
train_loss_list = []
dev_loss_list = []

with open(log_path, 'r') as f:
    for line in f:
        line = line[:-1]
        if line != model_path:
            continue
        
        for line in f:
            if line[0] == "*":
                break
            info = line.split(": ")
            if info[0] not in ["Train", "DEV"]:
                continue
            mtype = info[0]
            total_loss = float(info[-1].split(" / ")[0])
            
            if mtype == "Train":
                train_loss_list.append(total_loss)
            if mtype == "DEV":
                dev_loss_list.append(total_loss)
        break

with open(out_path, 'w') as f:
    f.write("epoch,Train,Dev\n")
    for epoch in range(len(train_loss_list)):
        f.write(str(epoch)+",")
        f.write(str(train_loss_list[epoch])+",")
        f.write(str(dev_loss_list[epoch])+",")
        f.write('\n')



