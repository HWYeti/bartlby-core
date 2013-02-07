#!/bin/sh

MY_NAME=$(basename $0);
MY_NAME=${MY_NAME/./}


function getConfigValue {
	if [ ! -f $BARTLBY_CONFIG ];
	then
		echo "Config $BARTLBY_CONFIG doesnt look like a file"  3
		exit;
	fi;

	r=`cat $BARTLBY_CONFIG |grep $1|awk -F"=" '{print \$2}'`
	echo $r;

}


RRD_HTDOCS=$(getConfigValue performance_rrd_htdocs);

RRD_TOOLCFG=$(getConfigValue rrd_tool|tr -d [:blank:]);
RRDTOOL=${RRD_TOOLCFG:-'/usr/bin/rrdtool'}

RRDFILE="${RRD_HTDOCS}/${1}_${MY_NAME}.rrd";





DS_STR="";
UPD_STR="";
GRAPH_STR="";
cnt=1;
LEGEND_STR="";

cl[1]="#ff0000";
cl[2]="#00ff00";
cl[3]="#0000ff";


function rrd_getDS {
			
			rrdtool info $1  |grep "ds\["|awk -F"." '{gsub(/ds\[/, "", $1); gsub(/\]/, "", $1); DS[$1]=1;} END {for(x in DS) { print x}}'|while read k; 
			do
				
				
				GRAPH_STR="DEF:report${cnt}=${RRDFILE}:${k}:AVERAGE LINE${cnt}:report${cnt}${cl[$cnt]}:${k}"
				cnt=$[cnt+1];
				echo $GRAPH_STR;
			done;

}

if [ "x$1" = "xgraph" ];
then
	
		RRDFILE="${RRD_HTDOCS}/${2}_${MY_NAME}.rrd";
		PNGFILE="${RRD_HTDOCS}/${2}_${MY_NAME}.png";
		PNGFILE_SEVEN="${RRD_HTDOCS}/${2}_${MY_NAME}7.png";
		PNGFILE_MONTH="${RRD_HTDOCS}/${2}_${MY_NAME}31.png";
		PNGFILE_YEAR="${RRD_HTDOCS}/${2}_${MY_NAME}365.png";
		PNGFILE_HOUR="${RRD_HTDOCS}/${2}_${MY_NAME}60min.png";

		if [ ! -f $RRDFILE ];
		then
			echo "NO RRD DB exists";
			exit;
		fi;
		
	
		GRAPH_STR=$(rrd_getDS $RRDFILE);		
		
	
		rrdtool graph $PNGFILE_HOUR  --start -1hour \
		-a PNG \
		--vertical-label "Generic PerfGraph" \
		 -w 600 -h 300 \
		$GRAPH_STR \
		COMMENT:"\\n"  
		
		
		rrdtool graph $PNGFILE  --start -86400 \
		-a PNG \
		--vertical-label "Generic PerfGraph" \
		 -w 600 -h 300 \
		$GRAPH_STR \
		COMMENT:"\\n"  
		
		rrdtool graph $PNGFILE_SEVEN  --start -1week \
		-a PNG \
		--vertical-label "Generic PerfGraph" \
		 -w 600 -h 300 \
		$GRAPH_STR \
		COMMENT:"\\n"  
		
		
		rrdtool graph $PNGFILE_MONTH  --start -1month \
		-a PNG \
		--vertical-label "Generic PerfGraph" \
		 -w 600 -h 300 \
		$GRAPH_STR \
		COMMENT:"\\n"  
		
		
		rrdtool graph $PNGFILE_YEAR  --start -1month \
		-a PNG \
		--vertical-label "Generic PerfGraph" \
		 -w 600 -h 300 \
		$GRAPH_STR \
		COMMENT:"\\n"  
		
		
		echo "done $PNGFILE_HOUR";
		
		exit;
				
fi;

cnt=1;
for x in $*;
do
	if [ "$x" = "last" ];
	then
		continue;
	fi;
	if [ "$x" = "PERF:" ];
	then
		continue;
	fi;
	if [ "$x" = "$1" ];
	then
		continue;
	fi;
	
	k=$(echo $x| awk -F"=" '{print $1}');
	v=$(echo $x| awk -F"=" '{print $2}');
	DS_STR="${DS_STR} DS:${k}:GAUGE:600:0:U";
	UPD_STR="${UPD_STR}${v}:";
	cnt=$[cnt+1];

done

if [ ! -f $RRDFILE  ];
then 
rrdtool create $RRDFILE $DS_STR \
RRA:AVERAGE:0.5:1:800 \
RRA:AVERAGE:0.5:6:800 \
RRA:AVERAGE:0.5:24:800 \
RRA:AVERAGE:0.5:288:800 \
RRA:MAX:0.5:1:800 \
RRA:MAX:0.5:6:800 \
RRA:MAX:0.5:24:800 \
RRA:MAX:0.5:288:800 \
RRA:MIN:0.5:1:800 \
RRA:MIN:0.5:6:800 \
RRA:MIN:0.5:24:800 \
RRA:MIN:0.5:288:800 \
RRA:LAST:0.5:1:800 \
RRA:LAST:0.5:6:800 \
RRA:LAST:0.5:24:800 \
RRA:LAST:0.5:288:800




fi;
rrdtool update $RRDFILE N:${UPD_STR%?};


