#/bin/sh
for i in {0..5}
do
./myftpclient clientconfig.txt get file$i &
done
