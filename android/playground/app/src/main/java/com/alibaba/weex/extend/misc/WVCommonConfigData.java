package com.alibaba.weex.extend.misc;




public class WVCommonConfigData {
	public String v = "0";//默认值 0
	public double packagePriorityWeight = 0.1;//
	public long updateInterval = 300000;//默认值5分钟
	public int packageAppStatus = 2;//类型 int 0-2，0 表示关闭 PackageApp 和 ZCache，1 表示打开但不更新配置，2 表示打开且更新配置 
	public int monitorStatus = 2;//类型 int 0-2，0 表示关闭 Monitor，1 表示打开但不更新配置，2 表示打开且更新配置
	public int urlRuleStatus = 2;//类型 int 0-2，0 表示关闭 urlRule，1 表示打开但不更新配置，2 表示打开且更新配置
	public String urlScheme = "http";//默认补全scheme时使用 http
	public String verifySampleRate = null;//安全校验采样
	public double ucsdk_alinetwork_rate = 1.0;
	public double ucsdk_image_strategy_rate = 1.0;//图片库使用率
	public boolean useSystemWebView =false;
	public String cookieUrlRule="";
	public String ucCoreUrl= ""; //ucsdk更新包
	public boolean isOpenCombo = true;
	public boolean isCheckCleanup = true; //packageApp是否校验cleanUp
	public String ucParam = "skip_old_extra_kernel=true";
	public String shareBlankList ="";//分享功能 黑名单
	public int packageDownloadLimit = 30;//默认限制
	public int packageAccessInterval = 3000;//计数间隔(3s)
	public int packageRemoveInterval = 432000000;//默认清除时间 5天
	public int recoveryInterval = 432000000;//默认还原时间 5天
	public int customsDirectQueryLimit = 10;//自主拉取custom配置限制
	public int customsComboLimit = 3;//combo 数量
	public String packageZipPrefix = ""; //zipPrefix
	public String packageZipPreviewPrefix = "";//预览地址
	public boolean isAutoRegisterApp = false; //packageApp是否自动安装
	public boolean isUseTBDownloader = true; //是否使用tbdownloader
	public boolean isUseAliNetworkDelegate = true; //是否使用AliNetworkDelegate
	public String excludeUCVersions = "1.12.11.0, 1.15.15.0, 1.14.13.0, 1.13.12.0";//不加载的uc 内核版本名单
	public int packageMaxAppCount = 100;//预加载app最大安装数
	public int zipDegradeMode = 0; //0:关闭 1:降级到ZipInputStream 2:直接关闭预加载
	public String zipDegradeList = ""; //e.g: HUAWEI@7.0
  public boolean ucSkipOldKernel = true; //初始化时跳过旧内核

	public boolean needZipDegrade = false;
}