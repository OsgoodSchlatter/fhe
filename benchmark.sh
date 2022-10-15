./alice 1 2 3 4 5 6 7 8 9
echo "division" >> div_mult.txt
for i in {2..20}
do
    echo "--${i} inputs" >> div_mult.txt
    for j in {1..5}
    do
        echo $(./cloud ${i} 2) >> div_mult.txt
    done
done

    