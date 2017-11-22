## Documentation
[C++ FlexLayout API Documentation](https://lark.alipay.com/weex/weexcore/gsg31z)

## Testing
For testing we rely on [gtest](https://github.com/google/googletest) as a submodule. After cloning weex run `git submodule init` followed by `git submodule update`.

For coverage we rely on [lcov](https://github.com/linux-test-project/lcov) as a submodule. After cloning weex run `git submodule init` followed by `git submodule update`.

For any changes you make you should ensure that all the tests are passing. In case you make any fixes or additions to the library please also add tests for that change to ensure we don't break anything in the future. Tests are located in the `${currentDir}/case` directory. Run the tests by executing `sh ${currentDir}/runTest.sh`.

## USE IDE 
### Xcode
We can use Xcode for development. use command like this:

```
mkdir build
cd build
cmake .. -DTEST=ON -DCMAKE_BUILD_TYPE=Release -G Xcode
```
then you can found `WeexCore_FlexLayout.xcodeproj` in `build` dir, open it in Xcode.