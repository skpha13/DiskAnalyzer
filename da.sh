#!/bin/bash
if [ $# -ne 1 ]       								#check the number of parameters
then
	case $# in
	
		0)
		echo -e "You must specify the operation you want to make ( add | rem )."
		;;
			
		*) 
		echo "Too many arguments."
		;;
	esac
else
	DESTINATION='/home/'$USER'/.bashrc'   					#where I put the .exe file
	OUR_EXEC_LOCATION='#Add DiskAnalyzer to PATH \nexport PATH="'$(pwd)':$PATH"\n'  #the needed command + location of our .exe file
	case $1 in								#check the parameter
	
		add)								#add the file
		
		echo -e $OUR_EXEC_LOCATION >> $DESTINATION			#append 
		echo "Successfully added."
		;;
		
		
		
		
		
		rem) 								#remove the file
		
		positions=$(echo -e $(grep -n "$(echo -e $OUR_EXEC_LOCATION)" $DESTINATION | cut -d : -f 1)) 	#get the numbers of
		 								#the lines where I find the OUR_EXEC_LOCATION variables
		
		start=$(echo $positions | cut -d " " -f 1)			#take the first index
		finish=$(echo $positions | tr " " "\n" | tail -n 1)		#take the last one
		
		if [[ -z $start || -z $finish ]]				#check if empty
		then
			echo "The file does not exist."
		else
			sed -i $start,$finish'd' $DESTINATION			#for the interval [start, finish], delete all the lines
			echo "Successfully removed."
		fi
		;;
		
		
		
		*)
		echo "Invalid parameter."
	esac
	
fi