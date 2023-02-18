#!/bin/bash
echo Enter GCP Project Name:
read project_name

echo Enter Tag Name:
read tag

curr_dir=$(cd -P -- "$(dirname -- "$0")" && pwd -P)
parent_dir=$(dirname $curr_dir)

echo Building image...
docker build $parent_dir -t jpoode_ppm-image:$tag -f "${parent_dir}/Dockerfile"

echo Image built, tagging...
docker tag jpoode_ppm-image:$tag us.gcr.io/$project_name/jpoode_ppm-image:$tag

echo Pushing image to repository...
docker -- push us.gcr.io/${project_name}/jpoode_ppm-image:$tag