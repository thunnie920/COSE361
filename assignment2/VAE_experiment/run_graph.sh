#!/bin/bash

log_path=add train log.txt
out_dir=graph

mkdir -p $out_dir

#for mtype in VAE1 VAE2 VAE3 MD; do
for mtype in VAE3; do
    python3 print_graph.py --log_path=$log_path --model_path=model/${mtype} --out_path=${out_dir}/${mtype}.csv
done

# for mtype in SI I LI D1 D2 AC CC SC HCP2 HCP3 HCP4 DEC1 DEC2 DEC3 DEC4 more; do
#     python3 print_graph.py --log_path=$log_path --model_path=model/VAE3_${mtype}_1 --out_path=${out_dir}/${mtype}.csv
# done
