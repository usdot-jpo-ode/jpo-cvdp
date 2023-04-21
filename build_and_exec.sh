# format of tag: 01-01-2020t12.00pm (lowercase t, am, pm)
tag=$(date +"%m-%d-%Yt%I.%M%p" | tr '[:upper:]' '[:lower:]')
echo "Building ppm-test-$tag"
docker build . -t ppm-test-$tag

echo "Running ppm-test-$tag"
docker run -it ppm-test-$tag /bin/bash