========================================================================
    memeclust: Quote extractiong and clustering application
========================================================================

The application can 
1. Construct meme quote base by parsing the spinn3r rar files on Zarya and cluster quotes in it, dump the result.
2. Construct meme quote base by reading quotes from input file and cluster quotes in it, dump the result.
3. Construct meme quote base by reading timestamped quotes from input file and cluster quotes in it, dump the result. 

Depending on the platform (Windows or Linux) you need to edit the Makefile.

/////////////////////////////////////////////////////////////////////////////
Parameters:
	-do: Which application to run ("memeclustzarya" for application 1, "memeclustqtonly" for application 2, "memeclustqttime" for application 3)
	-i:	Input file containing path and name of Zarya Memes dataset files(application 1) or a file containing quotes(application 2) or file containing <time>\t<quotes> in each line(application 3)
	-u:	File with news media urls	(default:"news_media.txt")
	-o:	Output file name prefix
	-minl: Min quote word length (default:4)
	-maxl: Max quote word length (default:200)
	-f:	Min meme frequency		(default:5)
	-cf:  Min quote cluster frequency (default:50)
	-b:	 Quote blacklist file name (default:"quote_blacklist.txt")
	-qtbsready: Indicate whether quote base is ready and can be loaded readily (default:false);
	-shglready: Indicate whether shingle hash table is ready. When tuning paramters, you may not want to construct the shingle hash table from scratch, so just load result of previous run. (default:false)
	-netready:  Indicate whether cluster net is ready. When you want to load the clusting result from previous run, set it to true. (default:false);
	-bktthresh: Threshold to decide whether a shingle hash table bucket is good or not, higher the value, higher the recall (default:0.4)
	-delaythresh: Max median time delay between two quote variants in the same cluster (default:5(days))
	-devthresh: Time deviation for a quote to decide whether the quote is a specific quote or a general quote (default:3(days))
	-refinethresh: Threshold for merging quote cluster in refining process, higher the value, higher the recall (default:0.2)
	-mf:  Min meme frequency (default:5)

	-mint:	Min time of quotes, format = YYYYMMDD	
	-maxt:	Max time of quotes, format = YYYYMMDD

/////////////////////////////////////////////////////////////////////////////
Usage:

Application 1: Meme clustering from Zarya data files
Cluster quotes from 2011 Jan to 2011 Jun on Zarya
memeclust -do:memeclustzarya -i:201101_201106.txt -u:news_media.txt -o:11011106 -mint:20110101 -maxt:20110701

Application 2: Meme clustering from input file with a list of quotes, one quote per line
memeclust -do:memeclustqtonly -i:input.txt -o:result

Application 3: Meme clustering from input file with a list of quotes with timestamp, format: <time>\t<quote> for each line
memeclust -do:memeclustqttime -i:input.txt -o:result -mint:20110101 -maxt:20110701
