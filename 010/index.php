<?php
#$homepage = file_get_contents('http://www.ndbc.noaa.gov/data/latest_obs/46216.rss'); #Goleta Bouy, not working right now
$homepage = file_get_contents('http://delasantafe.cl/rss/46217.php'); #Anacapa Passage
echo $homepage;
?>
##http://www.ndbc.noaa.gov/data/latest_obs/46216.rss <- Goleta Point Buoy
