cd ../
rm -f atlas_test.tar atlas_test.tar.bz2
tar --exclude 'CVS' -c -f atlas_test.tar AtlasTest
bzip2 atlas_test.tar
cd AtlasTest
