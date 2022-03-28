# Testcases

## Test schedule alarm in future, past and check ringing

Schedule an alarm in the past, this should not be allowed. Then schedule an alarm in 30 seconds and wait 30 seconds for ringining message and sound to appear.
<br>Example below
```
$ ./alarmclock
Welcome to the alarm clock! It is currently Fri Feb 11 12:26:19 2022
 
Please enter "s" (schedule), "l" (list), "c" (cancel), "x" (exit) 
>s
Schedule alarm at which date and time?
Format -> 'YYYY-MM-DD_hh:mm:ss'. Type 'c' to cancel
> 2022-02-11_12:26:00
Alarm has to be scheduled in the future, please try again
> 2022-02-11_12:27:51
Successfully created an alarm at Fri Feb 11 12:27:51 2022
 
Will RingeDing in 30 seconds
...

Ringeling tingting
```
<br><br>

## Test schedule alarm when capacity is reached

Schedule 10 alarms, then try to schedule another. This should not be possible because the maximum number of alarms is set to 10.
<br>Example below
<br>
```
$ ./alarmclock
Welcome to the alarm clock! It is currently 2022-02-11 10:20:00
Please enter "s" (schedule), "l" (list), "c" (cancel), "x" (exit)
> s
Schedule alarm at which date and time?
Format -> 'YYYY-MM-DD_hh:mm:ss'. Type 'c' to cancel 
> 2022-03-11_11:20:00
Successfully created an alarm at Fri Feb 11 11:20:00 2022

Will RingeDing in 3600 seconds
...
(repeat this 9 more times)

> s
No more space for alarms. Please cancel an alarm

```
<br><br>

## Test schedule and list, then delete and list

Test list functionality by scheduling 3 alarms and listing them.
Then delete one of the scheduled alarms and list again. The deleted alarms should not appear. 
<br>Example below
<br><br>
```
$ ./alarmclock
Welcome to the alarm clock! It is currently Fri Feb 11 12:32:43 2022
 
Please enter "s" (schedule), "l" (list), "c" (cancel), "x" (exit) 
> s

Schedule alarm at which date and time?
Format -> 'YYYY-MM-DD_hh:mm:ss'. Type 'c' to cancel
> 2022-02-11_13:00:00
Successfully created an alarm at Fri Feb 11 13:00:00 2022
 
Will RingeDing in 1634.000000 seconds
> s

Schedule alarm at which date and time?
Format -> 'YYYY-MM-DD_hh:mm:ss'. Type 'c' to cancel
> 2022-02-11_13:00:00
Successfully created an alarm at Fri Feb 11 13:00:00 2022
 
Will RingeDing in 1632.000000 seconds
> s

Schedule alarm at which date and time?
Format -> 'YYYY-MM-DD_hh:mm:ss'. Type 'c' to cancel
> 2022-02-11_13:00:00
Successfully created an alarm at Fri Feb 11 13:00:00 2022
 
Will RingeDing in 1631.000000 seconds
> l
Alarm 0 at Fri Feb 11 13:00:00 2022
Alarm 1 at Fri Feb 11 13:00:00 2022
Alarm 2 at Fri Feb 11 13:00:00 2022
> c
Cancel which alarm? 1
> l
Alarm 0 at Fri Feb 11 13:00:00 2022
Alarm 2 at Fri Feb 11 13:00:00 2022
```
## Test schedule and exit, then check for zombies

Schedule 3 alarms, then exit the program with 'x'. after the program has exited, check for zombies with ps aux | grep 'Z'. There should be no zombies because all processes are killend and then waited for.
<br> Example below
```
$ ./alarmclock
Welcome to the alarm clock! It is currently 2022-02-11 10:20:00
Please enter "s" (schedule), "l" (list), "c" (cancel), "x" (exit)
> s
Schedule alarm at which date and time?
Format -> 'YYYY-MM-DD_hh:mm:ss'. Type 'c' to cancel 
> 2022-03-11_11:20:00
Successfully created an alarm at Fri Feb 11 11:20:00 2022

Will RingeDing in 3600 seconds

...
(repeat this 2 more times)

```
```
ps aux | grep 'Z'
USER       PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
stianjsu  8683  0.0  0.0   8168   668 pts/4    S+   11:28   0:00 grep--color=auto Z
```


