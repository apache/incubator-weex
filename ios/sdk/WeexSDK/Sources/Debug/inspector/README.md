weex debugger server安装步骤：

1.安装ponyd script到~/Library/PonyDebugger/bin/ponyd路径下，并加入到全局环境变量

$：curl -sk https://cloud.github.com/downloads/square/PonyDebugger/bootstrap-ponyd.py | \
  python - --ponyd-symlink=/usr/local/bin/ponyd ~/Library/PonyDebugger


2.执行如下命令启动服务器

$：ponyd serve —listen-interface=localhost

此时打开浏览器输入

http://localhost:9000开始调试

3.如果真机调试localhost改为本机ip

注：如果第一步执行错误，无响应或是执行之后无法执行pondy命令，尝试手动安装，执行步骤如下：

1.下载安装脚本:

$：curl -O https://cloud.github.com/downloads/square/PonyDebugger/bootstrap-ponyd.py

2.安装服务器
$：python bootstrap-ponyd.py --ponyd-symlink=/usr/local/bin/ponyd ~/Library/PonyDebugger

注：如果此步安装失败，借助virtualenv工具， virtualenv工具是独立的python执行环境，执行如下命令：$VENV是你要安装的目标路径
$：sudo easy_install virtualenv

$：virtualenv “$VENV"

$：source “$VENV/bin/activate"

//通过pip安装ponyDebugger
$：pip install -e git+https://github.com/Eichhoernchen/pybonjour.git#egg=pybonjour

//确认pondy是否存在
$：hash -r

此时重新执行命令：
$：python bootstrap-ponyd.py --ponyd-symlink=/usr/local/bin/ponyd ~/Library/PonyDebugger

如果以上命令出错信息如下：
Could not find a version that satisfies the requirement pybonjour==1.1.1 (from ponyd) (from versions: )
No matching distribution found for pybonjour==1.1.1 (from ponyd)

执行以下命令：

$：git clone git@github.com:square/PonyDebugger.git
$：cd PonyDebugger
$：python setup.py install

此时重新执行命令：
$：python bootstrap-ponyd.py --ponyd-symlink=/usr/local/bin/ponyd ~/Library/PonyDebugger

