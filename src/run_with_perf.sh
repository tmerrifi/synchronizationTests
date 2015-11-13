
events=`cat counters | awk '{printf $1",";}END{printf "cycles"}'`;

#ocperf.py stat -e $events -a -C $1 $2;

ocperf.py stat -A -e $events -a $2;
