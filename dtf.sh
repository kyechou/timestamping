#!/bin/bash

set -e

SCRIPT_DIR="$(dirname $(realpath ${BASH_SOURCE[0]}))"
cd "$SCRIPT_DIR"

python tfbench/scripts/tf_cnn_benchmarks/tf_cnn_benchmarks.py \
	--device=gpu \
	--local_parameter_device=gpu \
	--num_intra_threads=0 \
	--batch_size=128 \
	--model=resnet50 \
	--variable_update=parameter_server \
	--ps_hosts=mustang03:6000 \
	--worker_hosts=mustang04:6001 \
	$@

#	--data_format=NHWC \
#	--job_name=ps \
#	--task_index=0
