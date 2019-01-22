#!/bin/bash

SCRIPT_DIR="$(dirname $(realpath ${BASH_SOURCE[0]}))"
cd "$SCRIPT_DIR/tfbench/scripts/tf_cnn_benchmarks"

python tf_cnn_benchmarks.py \
	--device=cpu \
	--local_parameter_device=cpu \
	--data_format=NHWC \
	--num_intra_threads=0 \
	--batch_size=128 \
	--model=resnet50 \
	--variable_update=parameter_server \
	--ps_hosts=192.17.151.31:6000 \
	--worker_hosts=128.174.240.171:6000 \
	$@

#	--job_name=ps \
#	--task_index=0
