
for i in {1..5};
do 
./myftpclient clientconfig.txt put file$i &
done

