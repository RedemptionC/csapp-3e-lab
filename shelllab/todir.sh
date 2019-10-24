for file in ./*
do
	if test -f $file
	then
		mv $file ./shelllab
	fi
	
done
