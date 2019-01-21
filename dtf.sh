#!/bin/bash

SCRIPT_DIR="$(dirname $(realpath ${BASH_SOURCE[0]}))"
cd "$SCRIPT_DIR/tfbench/scripts/tf_cnn_benchmarks"

python tf_cnn_benchmarks.py \
	--local_parameter_device=cpu \
	--data_format=NHWC \
	--batch_size=64 \
	--model=trivial \
	--variable_update=distributed_replicated \
	--ps_hosts=192.17.151.31:6000 \
	--worker_hosts=128.174.240.171:6000,128.174.240.171:6001 \
	$@

#	--job_name=ps \
#	--task_index=0
