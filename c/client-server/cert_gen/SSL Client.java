package org.innoviti.socket;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.security.KeyStore;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.cert.X509Certificate;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import org.innoviti.portUtil.SerialCommResponse;
import org.innoviti.unipay.GloabalValueAndPath;
import org.innoviti.xmlutil.LogFile;

//import org.innoviti.portUtil.PortOpenUtility;

public class SocketUtil {

	static char[] hexChar = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
			'a', 'b', 'c', 'd', 'e', 'f' };

	public static String toHexString(byte[] b) {
		StringBuffer sb = new StringBuffer(b.length * 2);
		for (int i = 0; i < b.length; i++) {
			// look up high nibble char
			sb.append(hexChar[(b[i] & 0xf0) >>> 4]);

			// look up low nibble char
			sb.append(hexChar[b[i] & 0x0f]);
		}
		return sb.toString();
	}

	public static int timeOutMilli = 60000;

	public static SocketDetail getSocketDetail() {
		SocketDetail socketDetail = new SocketDetail();
		String ipAddress = "";
		int portNumber = 0;
		try {
			BufferedReader reader = new BufferedReader(new FileReader(new File(
					GloabalValueAndPath.CONFIG_IP_FILE)));
			String tempReadLine = "";

			ipAddress = reader.readLine();
			String portNumberString = reader.readLine();

			LogFile.writeLog("Ip address is " + ipAddress);
			LogFile.writeLog("port is " + portNumberString);

			if (ipAddress == null || portNumberString == null) {
				LogFile.writeLog("ip or port no read fails");
				// System.out.println("ip or port no read fails");
				return null;
			}
			portNumber = Integer.parseInt(portNumberString);

		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			LogFile.writeLog("ip or port no read FileNotFoundException");
			// System.out.println("ip or port no read FileNotFoundException");
			return null;
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			LogFile.writeLog("ip or port no read exception");
			// System.out.println("ip or port no read exception");
			return null;
		} catch (NumberFormatException e) {
			// TODO: handle exception

			LogFile.writeLog("Number format exception");
			// System.out.println("Number format exception");
			return null;
		}

		socketDetail.setIpAddress(ipAddress);
		socketDetail.setPortNumber(portNumber);

		return socketDetail;
	}

	public static Socket createSocket(String ipAddress, int portNumber) {
		int length = 10;
		try {

			LogFile.writeLog("Inside Socket create");

			System.setProperty("javax.net.ssl.trustStore",
					GloabalValueAndPath.KEY_FILE);// "C:/Documents and Settings/kasi/mySrvKeystore");
			System.setProperty("javax.net.ssl.trustStorePassword",
					"unipay123456");

			KeyStore keyStore;
			TrustManager[] trustAllCerts = new TrustManager[] { new X509TrustManager() {
				@Override
				public X509Certificate[] getAcceptedIssuers() {
					return null;
				}

				@Override
				public void checkClientTrusted(X509Certificate[] certs,
						String authType) {
					return;
				}

				@Override
				public void checkServerTrusted(X509Certificate[] certs,
						String authType) {
					return;
				}
			} };

			SSLContext sslContext = SSLContext.getInstance("ssl");
			sslContext.init(null, trustAllCerts, new SecureRandom());
			SSLSocketFactory sslSocketFactory = sslContext.getSocketFactory();

			SSLSocket socket = (SSLSocket) sslSocketFactory.createSocket(
					ipAddress, portNumber);
			if (!(socket.isConnected())) {
				throw new UnknownHostException();
			}
			LogFile.writeLog("Socket create success");
			return socket;

		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			LogFile.writeLog(e.toString());
			// e.printStackTrace();
			return null;
		} catch (IOException e) {
			// TODO Auto-generated catch block
			LogFile.writeLog(e.toString());
			// e.printStackTrace();
			return null;
		} catch (NoSuchAlgorithmException e) {
			// TODO Auto-generated catch block
			LogFile.writeLog(e.toString());
			// e.printStackTrace();
		} catch (Exception e) {
			// TODO Auto-generated catch block
			LogFile.writeLog(e.toString());
			// e.printStackTrace();
		}
		return null;
	}

	// added by prasad

	public static int sendReceiveSocket(Socket socket, byte[] sendIsoMsg,
			SerialCommResponse commResponse) {
		InputStream socketInputStream = null;
		OutputStream socketOutputStream = null;
		int retVal = -1;

		try {
			socket.setSoTimeout(timeOutMilli);
			socketOutputStream = socket.getOutputStream();
			socketOutputStream.write(sendIsoMsg);
			System.out.println(" send iso reqest " + toHexString(sendIsoMsg));

			System.out.println("sendIsoMsg : " + new String(sendIsoMsg));
			socketOutputStream.flush();
			LogFile.writeLog("wrote data on socket successfull");
			socketInputStream = socket.getInputStream();
			byte[] tempReceive = new byte[12];
			byte[] firstReceive = null;
			byte[] secondReceive = null;
			int firstRxLength = 0;
			int secondRxLength = 0;
			int receivePacketLength = 0;
			Thread.sleep(10);
			for (int i = 0; i < 2; i++) {
				if (i == 0) {
					System.out
							.println("Inside if : i = " + Integer.toString(i));
					tempReceive = null;
					int j = socketInputStream.available();
					LogFile.writeLog("available : " + j);
					if (j == 0) {
						j = 4;
					}
					tempReceive = new byte[j];
					firstRxLength = socketInputStream.read(tempReceive, 0,
							tempReceive.length);
					LogFile.writeLog("read length at first attempt I: "
							+ firstRxLength + " "
							+ StringParsingUtil.hexString(tempReceive));

					if ("61736875".equalsIgnoreCase(StringParsingUtil
							.hexString(tempReceive))) {
						LogFile.writeLog("Got ashu and sending OK");
						socketOutputStream.write("OK".getBytes());
						socketOutputStream.flush();
						i -= 1;
						System.out.println("i value is : "
								+ Integer.toString(i));
					} else {
						firstReceive = hex2byte(StringParsingUtil
								.hexString(tempReceive));
						LogFile.writeLog("read length at first attempt II  : "
								+ firstRxLength + " "
								+ StringParsingUtil.hexString(firstReceive));
					}
				} else {
					System.out.println("Inside else : i = "
							+ Integer.toString(i));
					LogFile.writeLog("second available byte length from socket : "
							+ socketInputStream.available());
					secondReceive = new byte[socketInputStream.available()];
					secondRxLength = socketInputStream.read(secondReceive, 0,
							secondReceive.length);
					System.out.println("Read Bytes second time = "
							+ Integer.toString(secondRxLength));
					if (secondRxLength == 0) {
						secondReceive = new byte[socketInputStream.available()];
						secondRxLength = socketInputStream.read(secondReceive,
								0, secondReceive.length);
						System.out.println("Read Bytes second time+ = "
								+ Integer.toString(secondRxLength));
					}
				}

			}

			byte[] combinedReceiveByte = StringParsingUtil.addTwobyteArray(
					firstReceive, secondReceive);

			commResponse.setResponceISO(combinedReceiveByte);

			retVal = 0;

			LogFile.writeLog("Responce packet setted to Responce ISO  "
					+ new String(combinedReceiveByte));
			LogFile.writeLog("Responce packet setted to Responce ISO  "
					+ toHexString(combinedReceiveByte));

		} catch (NumberFormatException e) {
			retVal = 4;
		} catch (SocketException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			retVal = 1;
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			retVal = 2;
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			retVal = 4;
		}

		try {
			socketInputStream.close();
			socketOutputStream.close();
		} catch (Exception e) {
			// TODO: handle exception
		}
		LogFile.writeLog(" clearing the key store properties from system properties  ");

		System.clearProperty("javax.net.ssl.trustStore");
		System.clearProperty("javax.net.ssl.trustStorePassword");

		LogFile.writeLog(" successfully cleared  key store properties from system properties   ");

		return retVal;
	}

	public static byte[] hex2byte(String s) {
		if (s.length() % 2 == 0) {
			return hex2byte(s.getBytes(), 0, s.length() >> 1);
		} else {
			throw new RuntimeException("Uneven number(" + s.length()
					+ ") of hex digits passed to hex2byte.");
		}
	}

	public static byte[] hex2byte(byte[] b, int offset, int len) {
		byte[] d = new byte[len];
		for (int i = 0; i < len * 2; i++) {
			int shift = i % 2 == 1 ? 0 : 4;
			d[i >> 1] |= Character.digit((char) b[offset + i], 16) << shift;
		}
		return d;
	}

	public static void closeSocket(Socket socket) {
		try {
			socket.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

}
