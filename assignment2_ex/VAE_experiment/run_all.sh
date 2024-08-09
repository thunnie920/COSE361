#!/bin/bash
mkdir -p stats

echo base start
for trial in `seq 0 0`; do
    #for mtype in MD VAE3 VAE2 VAE1; do
    for mtype in VAE3; do # vanila vae train
        python3 -u train_base.py --model_type=$mtype --seed=${trial} --model_dir=model/${mtype} || exit 1;
        python3 -u convert.py --model_type=$mtype --model_path=model/${mtype} || exit 1;
        python3 -u calculate/calculate.py --test_dir=result/${mtype} > stats/${mtype}_${trial}.txt || exit 1;
    done
    echo base finsh


    # for mtype in VAE3 MD; do
    #     if [ -f model/${mtype}_${trial} ]; 
    #         then rm -rf model/${mtype}_${trial}
    #     fi
    #     cp -r model/${mtype} model/${mtype}_${trial}
    # done


    # for mtype in VAE3 MD; do
    #     if [ -f model/${mtype} ]; 
    #         then rm -rf model/${mtype}
    #     fi
    #     cp -r model/${mtype}_${trial} model/${mtype}
    # done

    for mtype in CC; do # cycle-consistency loss train
        python3 -u train_further.py --model_type=VAE3 --conf=conf/${mtype}.json --seed=$trial --model_dir=model/VAE3_${mtype}_${trial}|| exit 1;
        python3 -u convert.py --model_type=VAE3 --model_path=model/VAE3_${mtype}_${trial} || exit 1;
        python3 -u calculate/calculate.py --test_dir=result/VAE3_${mtype}_${trial} > stats/VAE3_${mtype}_${trial}.txt || exit 1;
    done

    # for mtype in HCP4 DEC1 DEC2 DEC3 DEC4; do 
    #     python3 -u train_further.py --model_type=MD --conf=conf/${mtype}.json --seed=$trial --model_dir=model/VAE3_${mtype}_${trial}|| exit 1;
    #     python3 -u convert.py --model_type=MD --model_path=model/VAE3_${mtype}_${trial} || exit 1;
    #     python3 -u calculate/calculate.py --test_dir=result/VAE3_${mtype}_${trial} > stats/VAE3_${mtype}_${trial}.txt || exit 1;
    # done
    
    # python3 -u train_further.py --model_type=VAE3 --seed=$trial --model_dir=model/VAE3_more_${trial}|| exit 1;
    # python3 -u convert.py --model_type=VAE3 --model_path=model/VAE3_more_${trial} || exit 1;
    # python3 -u calculate/calculate.py --test_dir=result/VAE3_more_${trial} > stats/VAE3_more_${trial}.txt || exit 1;

    echo further finsh
done
