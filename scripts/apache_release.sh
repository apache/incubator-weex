rm -f commit-history.log
git log --pretty=format:"%s %h" --no-merges  "$2"..HEAD > commit-history.log
sed -i '' 's/^*//' commit-history.log
sed -i '' 's/^/* /' commit-history.log
sed -i '' -e '1i \
# Detail Commit' commit-history.log
cat CHANGELOG.md commit-history.log > RELEASE_NOTE.md

sh scripts/generate_apache_source.sh
java -jar ./scripts/apache-rat-0.12.jar apache_release_temp > RELEASE_AUDIT.LOG
cp -r apache_release_temp apache-weex-incubating-$1-src
tar -czf "apache-weex-incubating-$1-src.tar.gz" apache-weex-incubating-$1-src/
rm -f "apache-weex-incubating-$1-src.tar.gz.asc"
gpg --armor --output "apache-weex-incubating-$1-src.tar.gz.asc" --detach-sig "apache-weex-incubating-$1-src.tar.gz"
gpg --print-md SHA512 "apache-weex-incubating-$1-src.tar.gz" > "apache-weex-incubating-$1-src.tar.gz.sha512"
rm -rf apache-weex-incubating-$1-src

# cd apache_release_temp
# sh scripts/build_from_source.sh $2 $3