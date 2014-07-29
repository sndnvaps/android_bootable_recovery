python 版本2.7.X

为了能方便生成中文字体和相应的font.dat系统中需要安装 python PIL,Image,ImageMack 

错误原因及解决方法：

使用font_h.py生成font_cn_H_W.h 

错误信息：

```bash
./font_h.py 32
preparing data
enter font size
32
painting ascii
Traceback (most recent call last):
  File "./font_h.py", line 57, in <module>
    text.text((0, i*height_en), s[i], 255, font_en)
  File "/usr/lib/python2.7/dist-packages/PIL/ImageDraw.py", line 256, in text
    ink, fill = self._getink(fill)
  File "/usr/lib/python2.7/dist-packages/PIL/ImageDraw.py", line 144, in _getink
    if self.palette and not Image.isNumberType(ink):
AttributeError: 'module' object has no attribute 'isNumberType'
```
 错误信息分析：

错误原因是Image模块中没有对应的isNumberType()方法，而这个isNumberType(number)是用来判断number是否为数字类型

如果是数字类型，就返回True,否则返回False

临时解决方法：在ImageDraw.py文件中创建一个相同的函数，就叫IsNumberType(),使用内建方法isinstance(a,b)来实现

代码：

```
#
# test the number is number or not 
def IsNumberType(number):
	if number is str:
		return False
	if number is "":
		return False
	if isinstance(number,int):
		return True
	if isinstance(number,float):
		return True
	return False
```

再把出错的位置的代码如下：

```
 if self.palette and not Image.isNumberType(ink):
 ... 

  if self.palette and not Image.isNumberType(fill):
```

修改为：
```
    if self.palette and not IsNumberType(ink): 
    ... 
    if self.palette and not IsNumberType(fill):
```
修改好后，需要修改文件的权限为当前用户，还要添加读写权限，否则无法正常保存文件。


