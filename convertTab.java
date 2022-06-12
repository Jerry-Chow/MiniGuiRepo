import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * CCE生成的tab词库文件转成十六进制libmgi可用的头文件
 */
public class TabBytesToC {

	public static void main(String[] args) throws Exception {
		if (args.length > 0) {
			File file = new File(args[0]);
			if (file.exists()) {
				if (file.isFile()) {
					System.out.println("请指定目录，而不是文件");
				} else {
					File[] tempList = file.listFiles();
					// 文件排序
					List<File> fileList = Arrays.asList(tempList);
					Collections.sort(fileList, new Comparator<File>() {
						@Override
						public int compare(File o1, File o2) {
							if (o1.isDirectory() && o2.isFile())
								return -1;
							if (o1.isFile() && o2.isDirectory())
								return 1;
							return o1.getName().compareTo(o2.getName());
						}
					});
					if (tempList.length > 0)
						System.out.println("该目录下文件个数：" + tempList.length);
					for (int i = 0; i < tempList.length; i++) {
						if (tempList[i].isFile()) {
							System.out.println("文件：" + tempList[i]);
						}
						if (tempList[i].isDirectory()) {
							System.out.println("文件夹：" + tempList[i]);
						}
					}

					// 如果需要生成的.h文件存在，则先删除
					File fileOut = new File(args[0] + "/ime_tab_pinyin.h");
					if (fileOut.exists())
						fileOut.delete();

					for (int i = 0; i < tempList.length; i++) {
						if (tempList[i].isFile()) {
							tab2Txt(tempList[i]);
						}
					}
					if (tempList.length > 0)
						System.out.println(args[0] + "ime_tab_pinyin.h 生成成功");
				}
			}
		} else {
			System.out.println("本程序可以把指定目录下的.tab、.tab.lx和.tab.phr"
					+ "三个文件换成十六进制的头文件表示");
			System.out.println("请指定文件所在的目录，例如");
			System.out.println("java -jar TabBytesToC.jar /home/anruliu/pinyin/");
		}
	}

	private static void tab2Txt(File file) {
		String ext = file.getName().substring(file.getName().lastIndexOf(".") + 1);
		if (ext.equals("tab") || ext.equals("lx") || ext.equals("phr")) {
			try {
				FileInputStream fis = new FileInputStream(file);
				String fileNameData = new String();
				if (ext.equals("tab")) {
					fileNameData = "ime_tab";
				} else if (ext.equals("lx")) {
					fileNameData = "ime_tab_lx";
				} else if (ext.equals("phr")) {
					fileNameData = "ime_tab_phr";
				}
				java.io.ByteArrayOutputStream bos = new java.io.ByteArrayOutputStream();
				byte[] buff = new byte[1024];
				int len = 0;

				while ((len = fis.read(buff)) != -1) {
					bos.write(buff, 0, len);
				}

				// 得到图片的字节数组
				byte[] result = bos.toByteArray();
				System.out.println(file.getName() + " 字节长度：" + result.length + " 正在解析中...");

				if (null != file.getParent()) {
					byte2HexStr(file.getParent(), "ime_tab_pinyin", fileNameData, result);
				} else {
					byte2HexStr(".", "ime_tab_pinyin", fileNameData, result);
				}

				fis.close();
				bos.close();

			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	/**
	 * 实现字节数组向十六进制转换并保存
	 * 
	 * @param b 字节数组
	 * @return 十六进制字符串
	 */
	private static void byte2HexStr(String filePatch, String fileName,
			String fileNameData, byte[] b) {
		String hs = "  ";
		String stmp = "";
		FileWriter fwriter = null;
		try {
			fwriter = new FileWriter(filePatch + "/" + fileName + ".h", true);
			fwriter.write("static unsigned char " + fileNameData + "[] = {\n");
			for (int n = 0; n < b.length; n++) {
				System.out.print(n + "/" + b.length + "\r");
				stmp = (Integer.toHexString(b[n] & 0XFF));
				if (stmp.length() == 1) {
					hs = hs + "0x0" + stmp;
				} else {
					hs = hs + "0x" + stmp;
				}
				if (n == b.length - 1) {
					hs += "\n";
				} else if (n != 0 && (n + 1) % 8 == 0) {
					hs += ",\n  ";
				} else {
					hs += ", ";
				}
				fwriter.write(hs);
				hs = "";
			}
			fwriter.write("};\n\n");
		} catch (IOException ex) {
			ex.printStackTrace();
		} finally {
			try {
				fwriter.flush();
				fwriter.close();
				System.out.println("static unsigned char " + fileNameData + "[] 生成成功");
			} catch (IOException ex) {
				ex.printStackTrace();
			}
		}
	}
}