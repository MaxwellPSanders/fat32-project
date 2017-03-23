Joint project between David Theodore and Maxwell Sanders
This project will be used to open and read fat32 file systems using the command line. Much of it will look like the shell assignment when it comes to the interface and the string parsing used. 
This will compile for sure on Linux with g++ and then can be run with no parameters

Other than that the functionality will provide the ability to:
Open and close fat32 filesystems 
	open
	close
Read files inside the filesystems
	read <filename> <position> <number of bytes>
Provide some information about the filesystem
	info
Change directories within the filesystem
	cd <directory>
Print out contents of a directory
	ls <directory>
Prints the volume name of the file system image
	volume
