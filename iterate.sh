#/bin/bash

radius_values=(1 2 3 4 5 6 7 8 10 14 18 22 24 28 30)
neighbours_values=( 1 2 3 4 5 6 7 8 9 10 )
total=$((${#radius_values[@]}*${#neighbours_values[@]}))
start=`date +%s`
loops=0
for radius in "${radius_values[@]}"
do
    for neighbours in "${neighbours_values[@]}"
    do
        loops=$(($loops+1))
        echo "Round ${loops}/${total}"
        # $ "run --radius=$radius --neighbours=$neighbours"
        ./detect.o 0 --cascade=lbpcascades/lbpcascade_frontalface.xml --scale=1 --test --silent --radius=$radius --neighbours=$neighbours
        current=`date +%s`
	left_total_s=$((($current - $start)/loops*(total-loops)))
	left_h=$(($left_total_s/3600))
	left_min=$((($left_total_s-$left_h*3600)/60))
	left_s=$((($left_total_s-$left_h*3600-$left_min*60)))
	echo "Time left ${left_h}h ${left_min}min ${left_s}s"
    done
done
