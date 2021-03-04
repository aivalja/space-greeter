#/bin/bash

#radius_values=(1 2 3 4 5 6 7 8 10 14 18 22 24 28 30)
#neighbours_values=( 1 2 3 4 5 6 7 8 9 10 )
radius_values=(8 14)
neighbours_values=(4 5 6 7 8 9)
cascade_values=("lbpcascades/lbpcascade_frontalface.xml" "lbpcascades/lbpcascade_frontalface_improved.xml" "haarcascades/haarcascade_frontalface_default.xml" "haarcascades/haarcascade_frontalface_alt.xml" "haarcascades/haarcascade_frontalface_alt2.xml" "haarcascades/haarcascade_frontalface_alt_tree.xml")
scale_values=(7.5 10 12.5 15 20 30 40 50)
total=$((${#radius_values[@]}*${#neighbours_values[@]}*${#cascade_values[@]}*${#scale_values[@]}))
start=`date +%s`
loops=0
for scale in "${scale_values[@]}"
do
    for cascade in "${cascade_values[@]}"
    do
        for radius in "${radius_values[@]}"
        do
            for neighbours in "${neighbours_values[@]}"
            do
                loops=$(($loops+1))
                echo "Round ${loops}/${total}"
                # $ "run --radius=$radius --neighbours=$neighbours"
                ./detect.o 0 --cascade=$cascade --scale=$scale --test --silent --radius=$radius --neighbours=$neighbours
                current=`date +%s`
                left_total_s=$((($current - $start)/loops*(total-loops)))
                left_h=$(($left_total_s/3600))
                left_min=$((($left_total_s-$left_h*3600)/60))
                left_s=$((($left_total_s-$left_h*3600-$left_min*60)))
                echo "Time left ${left_h}h ${left_min}min ${left_s}s"
            done
        done
    done
done
