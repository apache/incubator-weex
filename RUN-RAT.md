# Run Apache-Rat

### Download Apache Rat
Download the binary from [Rat's website](https://creadur.apache.org/rat/download_rat.cgi), decompress it somewhere.   
We've tested using v0.12.

### Run Rat against Release files
Just execute in your termial:

> `$ ant -buildfile scripts/rat-ant-build.xml -lib path_to_the_folder_you_place_rat/apache-rat-0.12.jar`

See '`scripts/rat-ant-build.xml`' for details about rules we use.