# Lists all the mobs in a certain level range
# Defaults to the range [0,100]
# To use: gawk -f mob-level.awk level=20 maxlevel=40 *.mob
BEGIN { 
    RS="#"; 
    if (!level) level = 0; else level += 0;
    if (!maxlevel) maxlevel = 100; else maxlevel += 0;
}
{     
    match($0, /(-?[0-9]+) -?[0-9]+ -?[0-9]+ [0-9]+d[0-9]+\+[0-9]+/, arr);
    moblvl = arr[1] + 0
    if (level <= moblvl && moblvl <= maxlevel) {
	split($0, lines, "\n");
	print "L: " moblvl " Vnum: " $1 " Name: " lines[3];
	num++;
    }
}
END { print "Number of mobs [L" level ",L" maxlevel "] : " num; }