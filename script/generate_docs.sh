# sudo apt-get install doxygen
doxygen Doxyfile
rm -rf docs/search
mv docs/html/* docs/
