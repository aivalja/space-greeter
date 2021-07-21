#/bin/bash

radius_values=(1 3 5 7 8 10 20 30)
neighbours_values=(1 3 5 7 9)
cascade_values=("lbpcascades/lbpcascade_frontalface.xml" "lbpcascades/lbpcascade_frontalface_improved.xml" "haarcascades/haarcascade_frontalface_default.xml" "haarcascades/haarcascade_frontalface_alt.xml" "haarcascades/haarcascade_frontalface_alt2.xml" "haarcascades/haarcascade_frontalface_alt_tree.xml")
scale_values=(1 1.25 1.5 2 4 5 7.5)
dataset_values=("dup1" "dup2" "fb")
single="--single"
total=$((${#radius_values[@]}*${#neighbours_values[@]}*${#cascade_values[@]}*${#scale_values[@]}*${#dataset_values[@]}))
start=`date +%s`
loops=0
test_log="test_log.txt"
test_remaining_log="test_remaining_log.txt"
skipped=0
tests_arr=()
code=""


while read p
do
    tests_arr+=($p)
done < $test_log

for dataset in "${dataset_values[@]}" 
do
    python3 data-sorter.py /home/anssi/FERET/colorferet/dvd1/data/images/ /home/anssi/FERET/colorferet/dvd1/doc/partitions/${dataset}.txt > test.csv
    for scale in "${scale_values[@]}"
    do
        for cascade in "${cascade_values[@]}"
        do
            for radius in "${radius_values[@]}"
            do
                for neighbours in "${neighbours_values[@]}"
                do
                    code="$dataset+$scale+$cascade+$radius+$neighbours+$single"
                    #check if we have already done this test
                    if [[ " ${tests_arr[@]} " =~ " ${code} " ]]
                    then
                        # Test ran before
                        loops=$(($loops+1))
                        echo "Round ${loops}/${total}"
                        skipped=$((skipped+1))
                        echo "Skipped"
                    else
                        # Test not ran before
                        loops=$(($loops+1))
                        echo "Round ${loops}/${total}"
                        # $ "run --radius=$radius --neighbours=$neighbours"
                        ./detect.o 0 --cascade=$cascade --scale=$scale --test --silent --radius=$radius --neighbours=$neighbours --dataset=$dataset $single
                        current=`date +%s`
                        left_total_s=$((($current - $start)/(loops-skipped)*(total-loops)))
                        left_d=$(($left_total_s/(3600*24)))
                        left_h=$((($left_total_s-$left_d*3600*24)/3600))
                        left_min=$((($left_total_s-$left_h*3600-$left_d*3600*24)/60))
                        left_s=$((($left_total_s-$left_h*3600-$left_min*60-$left_d*3600*24)))
                        echo "Time left ${left_d} days ${left_h}h ${left_min}min ${left_s}s"
                        echo $left_total_s >> $test_remaining_log
                        echo $code >> $test_log
                    fi
                done
            done
        done
    done
done
