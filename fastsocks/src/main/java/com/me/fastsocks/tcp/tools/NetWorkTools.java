package com.me.fastsocks.tcp.tools;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.telephony.TelephonyManager;

import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.InterfaceAddress;
import java.net.NetworkInterface;
import java.util.Collections;
import java.util.Enumeration;
import java.util.List;

/**
 * 网络工具类
 *
 * @author zaki
 */
public class NetWorkTools {


	//没有网络连接
	public static final int NETWORN_NONE = 0;
	//wifi连接
	public static final int NETWORN_WIFI = 1;
	//手机网络数据连接类型
	public static final int NETWORN_2G = 2;
	public static final int NETWORN_3G = 3;
	public static final int NETWORN_4G = 4;
	public static final int NETWORN_MOBILE = 5;

	/**
	 * 获取当前网络连接类型
	 * @param context
	 * @return
	 */
	public static int getCurrentType(Context context) {
		//获取系统的网络服务
		ConnectivityManager connManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);

		//如果当前没有网络
		if (null == connManager)
			return NETWORN_NONE;

		//获取当前网络类型，如果为空，返回无网络
		NetworkInfo activeNetInfo = connManager.getActiveNetworkInfo();
		if (activeNetInfo == null || !activeNetInfo.isAvailable()) {
			return NETWORN_NONE;
		}

		// 判断是不是连接的是不是wifi
		NetworkInfo wifiInfo = connManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
		if (null != wifiInfo) {
			NetworkInfo.State state = wifiInfo.getState();
			if (null != state)
				if (state == NetworkInfo.State.CONNECTED || state == NetworkInfo.State.CONNECTING) {
					return NETWORN_WIFI;
				}
		}

		// 如果不是wifi，则判断当前连接的是运营商的哪种网络2g、3g、4g等
		NetworkInfo networkInfo = connManager.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);

		if (null != networkInfo) {
			NetworkInfo.State state = networkInfo.getState();
			String strSubTypeName = networkInfo.getSubtypeName();
			if (null != state)
				if (state == NetworkInfo.State.CONNECTED || state == NetworkInfo.State.CONNECTING) {
					switch (activeNetInfo.getSubtype()) {
						//如果是2g类型
						case TelephonyManager.NETWORK_TYPE_GPRS: // 联通2g
						case TelephonyManager.NETWORK_TYPE_CDMA: // 电信2g
						case TelephonyManager.NETWORK_TYPE_EDGE: // 移动2g
						case TelephonyManager.NETWORK_TYPE_1xRTT:
						case TelephonyManager.NETWORK_TYPE_IDEN:
							return NETWORN_2G;
						//如果是3g类型
						case TelephonyManager.NETWORK_TYPE_EVDO_A: // 电信3g
						case TelephonyManager.NETWORK_TYPE_UMTS:
						case TelephonyManager.NETWORK_TYPE_EVDO_0:
						case TelephonyManager.NETWORK_TYPE_HSDPA:
						case TelephonyManager.NETWORK_TYPE_HSUPA:
						case TelephonyManager.NETWORK_TYPE_HSPA:
						case TelephonyManager.NETWORK_TYPE_EVDO_B:
						case TelephonyManager.NETWORK_TYPE_EHRPD:
						case TelephonyManager.NETWORK_TYPE_HSPAP:
							return NETWORN_3G;
						//如果是4g类型
						case TelephonyManager.NETWORK_TYPE_LTE:
							return NETWORN_4G;
						default:
							//中国移动 联通 电信 三种3G制式
							if (strSubTypeName.equalsIgnoreCase("TD-SCDMA") || strSubTypeName.equalsIgnoreCase("WCDMA") || strSubTypeName.equalsIgnoreCase("CDMA2000")) {
								return NETWORN_3G;
							} else {
								return NETWORN_MOBILE;
							}
					}
				}
		}
		return NETWORN_NONE;
	}



	private static NetworkInfo getNetworkInfo(Context mContext){
		ConnectivityManager connectivityManager = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
		return connectivityManager.getActiveNetworkInfo();
	}

	public static boolean isNetWorkConnect(Context mContext){
		NetworkInfo activeNetInfo =  getNetworkInfo(mContext);
		return (activeNetInfo != null && activeNetInfo.isConnected());
	}


//	/**
//	 * 获了本机的IP地址
//	 * @param useIPv4
//	 * @return
//	 * @author: zhuqian
//	 */
//	public static String getIPAddress(boolean useIPv4) {
//		try {
//			List<NetworkInterface> interfaces = Collections.list(NetworkInterface.getNetworkInterfaces());
//			for (NetworkInterface intf : interfaces) {
//				List<InetAddress> addrs = Collections.list(intf.getInetAddresses());
//				for (InetAddress addr : addrs) {
//					if (!addr.isLoopbackAddress()) {
//						String sAddr = addr.getHostAddress().toUpperCase();
//						boolean isIPv4 = InetAddressUtils.isIPv4Address(sAddr);
//						if (useIPv4) {
//							if (isIPv4) {
//								return sAddr;
//							}
//						} else {
//							if (!isIPv4) {
//								int delim = sAddr.indexOf('%');
//								return delim<0 ? sAddr : sAddr.substring(0, delim);
//							}
//						}
//					}
//				}
//			}
//		} catch (Exception ex) {
//			ex.printStackTrace();
//		}
//		return "";
//	}

	/**
	 * 当前是否wifi
	 * @param mcontext
	 * @return
	 * @author: zhuqian
	 */
	public static boolean isConnectWifi(Context mcontext) {
		ConnectivityManager connMng = (ConnectivityManager) mcontext.getSystemService(Context.CONNECTIVITY_SERVICE);
		NetworkInfo netInf = connMng.getActiveNetworkInfo();
		return netInf != null && "WIFI".equalsIgnoreCase(netInf.getTypeName());
	}

	/**
	 * 是否使用ipv6地址
	 *
	 * @return
	 */
	public static boolean isUseIpv6(){
		try {
			NetworkInterface networkInterface;
			Enumeration<NetworkInterface> networkInterfaces = NetworkInterface.getNetworkInterfaces();
			boolean hasIpv4 = false;
			boolean hasIpv6 = false;
			while (networkInterfaces.hasMoreElements()) {
				networkInterface = networkInterfaces.nextElement();
				if (!networkInterface.isUp() || networkInterface.isLoopback()) {
					continue;
				}
				List<InterfaceAddress> interfaceAddresses = networkInterface.getInterfaceAddresses();
				for (int a = 0; a < interfaceAddresses.size(); a++) {
					InterfaceAddress address = interfaceAddresses.get(a);
					InetAddress inetAddress = address.getAddress();
					if (inetAddress.isLinkLocalAddress() || inetAddress.isLoopbackAddress() || inetAddress.isMulticastAddress()) {
						continue;
					}
					if (inetAddress instanceof Inet6Address) {
						hasIpv6 = true;
					} else if (inetAddress instanceof Inet4Address) {
						String addrr = inetAddress.getHostAddress();
						if (!addrr.startsWith("192.0.0.")) {
							hasIpv4 = true;
						}
					}
				}
			}
			if (!hasIpv4 && hasIpv6) {
				return true;
			}
		} catch (Throwable e) {
			e.printStackTrace();
		}
		return false;
	}

	/**
	 * 是否使用了VPN
	 * 通过判断本地网卡接口
	 * @return
     */
	public static boolean isVpnUsed() {
		try {
			Enumeration<NetworkInterface> niList = NetworkInterface.getNetworkInterfaces();
			if(niList != null) {
				for (NetworkInterface intf : Collections.list(niList)) {
					if(!intf.isUp() || intf.getInterfaceAddresses().size() == 0) {
						continue;
					}
					if ("tun0".equals(intf.getName()) || "ppp0".equals(intf.getName())){
						return true; // The VPN is up
					}
				}
			}
		} catch (Throwable e) {
			e.printStackTrace();
		}
		return false;
	}

}
