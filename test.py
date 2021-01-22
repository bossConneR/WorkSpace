import sensor,image,lcd  # import 相关库
import utime
import time
import KPU as kpu
from Maix import FPIOA,GPIO
from board import board_info
from fpioa_manager import fm
clock = time.clock()  # 初始化系统时钟，计算帧率
###OUTPUT NODE>>>
fm.register(33, fm.fpioa.GPIOHS1)
vout = GPIO(GPIO.GPIOHS1, GPIO.OUT, GPIO.PULL_NONE)

fm.register(31, fm.fpioa.GPIOHS2)
sout = GPIO(GPIO.GPIOHS2, GPIO.OUT, GPIO.PULL_NONE)
##############>>>

key_pin=16 # 设置按键引脚 FPIO16
fpioa = FPIOA()
fpioa.set_function(key_pin,FPIOA.GPIO7)
key_gpio=GPIO(GPIO.GPIO7,GPIO.IN)
fm.register(35, fm.fpioa.GPIOHS0)
key = GPIO(GPIO.GPIOHS0, GPIO.IN, GPIO.PULL_NONE)
last_key_state=1
key_pressed=0 # 初始化按键引脚 分配GPIO7 到 FPIO16
last_key_state2=1
key_pressed2=0 # 初始化按键引脚 分配GPIO7 到 FPIO16
FLAG = 1 #切换模式标志，1为人脸识别，0为口罩识别

color_R = (255, 0, 0)
color_G = (0, 255, 0)
color_B = (0, 0, 255)

#task_fd = kpu.load(0x500000) # 从flash 0x500000 加载人脸检测模型
#task_ld = kpu.load(0x600000) # 从flash 0x600000 加载人脸五点关键点检测模型
#task_fe = kpu.load(0x700000) # 从flash 0x700000 加载人脸196维特征值模型
task_fd = kpu.load('/sd/FD_222f3c1e2adc76d0d7441874b3c2f2d3.kmodel') # 从sd卡中加载人脸检测模型
task_ld = kpu.load('/sd/KP_chwise_222f3c1e2adc76d0d7441874b3c2f2d3.kmodel') # 从sd卡中加载人脸五点关键点检测模型
task_fe = kpu.load('/sd/FE_mbv1_0.5_222f3c1e2adc76d0d7441874b3c2f2d3.kmodel') # 从sd卡中加载人脸196维特征值模型

def check_key(): # 按键检测函数，用于在循环中检测按键是否按下，下降沿有效
    global last_key_state
    global key_pressed
    val=key_gpio.value()
    if last_key_state == 1 and val == 0:
        key_pressed=1
    else:
        key_pressed=0
    last_key_state = val

def check_key2(): # 按键检测函数，用于在循环中检测按键是否按下，下降沿有效
    global last_key_state2
    global key_pressed2
    val2=key.value()
    if last_key_state2 == 1 and val2 == 0:
        key_pressed2=1
    else:
        key_pressed2=0
    last_key_state2 = val2

def drawConfidenceText(image, rol, classid, value):
    text = ""
    _confidence = int(value * 100)
    if classid == 1:
        text = 'mask: ' + str(_confidence) + '%'
    else:
        text = 'no_mask: ' + str(_confidence) + '%'
    image.draw_string(rol[0], rol[1], text, color=color_R, scale=2.5)

lcd.init() # 初始化lcd
sensor.reset() #初始化sensor 摄像头
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.set_hmirror(0) #设置摄像头镜像
sensor.set_vflip(1)   #设置摄像头翻转
sensor.run(1) #使能摄像头

anchor = (1.889, 2.5245, 2.9465, 3.94056, 3.99987, 5.3658, 5.155437, 6.92275, 6.718375, 9.01025) #anchor for face detect 用于人脸检测的Anchor
anchorMsk = (0.1606, 0.3562, 0.4712, 0.9568, 0.9877, 1.9108, 1.8761, 3.5310, 3.4423, 5.6823)
dst_point = [(44,59),(84,59),(64,82),(47,105),(81,105)] #standard face key point position 标准正脸的5关键点坐标 分别为 左眼 右眼 鼻子 左嘴角 右嘴角
a = kpu.init_yolo2(task_fd, 0.5, 0.3, 5, anchor) #初始化人脸检测模型
img_lcd=image.Image() # 设置显示buf
img_face=image.Image(size=(128,128)) #设置 128 * 128 人脸图片buf
a=img_face.pix_to_ai() # 将图片转为kpu接受的格式
record_ftr=[] #空列表 用于存储当前196维特征

#空列表 用于存储按键记录下人脸特征， <<<!可以将特征以txt等文件形式保存到sd卡后，读取到此列表，即可实现人脸断电存储!
record_ftrs=[bytearray(b"WQ\xc5Q\x1d\xbc\x007\xa4\xef\xd6\xfd\xce\xd3\xd3\x08\xdb\x1f\x11=\x19\x05\x1d\x12%!\xde\xfa)\x02\xf11\xbd\xa1\xb4\xf4\n5A\x16\xf6\xc1\xee\x00\x15F\x0fQ\xaa\t\x02\x13\x12\n\xcdW\x07>0\x04\xea\xec%\xc4\x0f\xf3\xf1\xf2\xd7%\xc6\x12\xe6\n'\xd82F+\xd3\xf9\xe7\xf8:\xd5\x07\x0e\x03\xee\xf6\x00\xdd*\xf3\xfe\xfe\x06\x14\xe4\xe4&72\x00\x0b\x05(2\xec4\xe6\xb7\xab\x10\x0f\x10\x1a\x0bF\x05\x0e\xd4\xf3\xe0\xe9\xe2\xf3)\xf6\xe6\xff(\xed4\x1a\xe5\xfe\x14\x16\xef\x10\xde\xd7\x11\x16\xdc\xfe\xf9\x0f\n\r\xfb\xde\x13\xfc\xee\x02%\xce<\x07\x1f\x0c\x114\xfe\xf5 \xfa\x114\t\x01\xf7\x02\xf0\xfb\xd7\x17\x14\xdb\xf8\x0e\x15\xe0\x00\xe0\x0c\xf3\xe7\x08)"),bytearray(b"/\x07\xae[\x1d\xcc\x1e+\xdc\t\xee\xf5\xd9\xe0\x02\x0f\xd9\x17\x02\x1b$*\x01\xe5\x16\x1f\x08\xfaF\xfa\xd9\x12\xeb\x9c\xa3\x06\x04*5\x1c\x06\x00\x1b\xe7\x19[\x00\x0f\xb4\xec\x1b\xfc\xff\x0f\xdf[\tC\n\xff\xe8\xc82\xac\n\xd0\xee\x0f\xf0/\xb3\x1b\xd1\xf2\xf6\x040\x18\x10\x08\x15\xfd\x16X\t\x1c\xe6\x06\t\xfd\xde\xce\x02\xf1\x08\x03 \xde\xd2\xe7\x00\x02\t\x0e\xf2\xfd\x18@\xe5D\xe3\xbe\xa22\x02\xfb\xf4\x12O\xf8\x0f\xeb\xc4\x0f\x0c\x11\x03O\x06\xe6\x10\x12\xf5:*\xf1%\x0e\xeb\x14\x05\xeb\x0c'\x0c\xea\xfb\x1e'\x15\xfd\xf6\x1a\x11\xf6\xe0\xee\x18\xb48\x12\xfa\x18F\x06\x11\xfd\x12*\xf4(\xee\xee\x0c\xdb\xf5\x18\xbd\x03\x03\x07\xe2=\x16\x06.\xe0\x172\xb0\x04\x10")
,bytearray(b'\x1b\x05\xcc<\xe8\xc0\x19\xdd\xd8\xf8\xe6\xfd\xc3\x14\xe68\xe8\xe4\xd2\x1cA\xd8K\xe0\xf0U\x00\xd7\x01\xf0\x19\xf8\xe0\xc2\xc3\xda\x0b8\xff0\xfc\xc4\t0\x1e*\xe9\xf0\xbb\xbd\xe8\x10\x01\x00\x14?\xe5[\x13\x00\xd8\x0b\x1f\x10\xf3\xef\x04\xd8\x14\xf7\x000\xfa\xbc\xf4\xba\x0bQa\xf7\xe5\x00\xea\xfd\t\x1b\x1a \xdc\x00\xd2\xf3\x03\xe9\xf9\x16\xf8\x03\xf5\x17\t\x1c\x00\x1b\xd8\xf1\x00\x1c\xea(\x06\xb0\xde\xdb\xd8\xee\xf5\xf2\x1688\x05\xdc\n\xd9\xee\xd5\xfa\x08\x020\xfe\xfb)\x081\xdf\xf37\x03\x0e\xea\x144\xd5\xa7.\xfe;\x17\xc6\xf8 \xe9\xfa\xf7\x14\xeb\xf8\xf4\x1a\x0b\x08M\x1d\x08\x1c\x0e\xd9"\x1f\x0e\xe1\x1e\xce\xfc\'\xec\xe2\xe8\x02\xe9\x02\x00\xc0<\xe8\t\xef\x99\xa5\xfa')]
names = ['hhx', 'wsf', 'cjd', 'crewD', 'crewE', 'crewF', 'crewG', 'crewH', 'crewI' , 'crewJ'] # 人名标签，与上面列表特征值一一对应。
class_IDs = ['no_mask', 'mask']

VECTOR = 0
VOUT = 0
COUNT = 0
vout.value(0)

#sout:是否检测到人脸，周期faceCount=10fps，7次有检测到就发送高电平sout.value(1)
faceCount = 0
faceVector = 0
SOUT = 0
sout.value(0)

while(1): # 主循环
    COUNT += 1 #30fps一周期,也就是说报警高电平会持续30fps
    faceCount += 1
    if(COUNT>(25-1)):
        if(VECTOR < 12):
            VECTOR = 0
            COUNT = 0
            VOUT = 1
            vout.value(VOUT)
            print('safe')
        else:
            VECTOR = 0
            COUNT = 0
            VOUT = 0
            vout.value(VOUT)
            print('warning')

    if(faceCount>(25-1)):
        if(faceVector > 12):
            faceVector = 0
            faceCount = 0
            SOUT = 1
            sout.value(SOUT)
            print('face detected')
        else:
            faceVector = 0
            faceCount = 0
            SOUT = 0
            sout.value(SOUT)
            print('nothing detected')

    if(FLAG):

        #///人脸识别///#
        check_key() #按键检测
        check_key2() #按键检测
        img = sensor.snapshot() #从摄像头获取一张图片
        clock.tick() #记录时刻，用于计算帧率
        code = kpu.run_yolo2(task_fd, img) # 运行人脸检测模型，获取人脸坐标位置
        if key_pressed2 == 1: #如果检测到按键
            print("shifted.")
            key_pressed2 = 0 #重置按键状态
            FLAG = 0
            a = kpu.deinit(task_fe)
            a = kpu.deinit(task_ld)
            a = kpu.deinit(task_fd)
            task = kpu.load('/sd/mask.kmodel')
            a = kpu.init_yolo2(task, 0.5, 0.3, 5, anchorMsk) #初始化人脸检测模型
            continue
        if code: # 如果检测到人脸
            faceVector += 1
            for i in code: # 迭代坐标框
                # Cut face and resize to 128x128
                a = img.draw_rectangle(i.rect()) # 在屏幕显示人脸方框
                face_cut=img.cut(i.x(),i.y(),i.w(),i.h()) # 裁剪人脸部分图片到 face_cut
                face_cut_128=face_cut.resize(128,128) # 将裁出的人脸图片 缩放到128 * 128像素
                a=face_cut_128.pix_to_ai() # 将裁出图片转换为kpu接受的格式
                #a = img.draw_image(face_cut_128, (0,0))
                # Landmark for face 5 points
                fmap = kpu.forward(task_ld, face_cut_128) # 运行人脸5点关键点检测模型
                plist=fmap[:] # 获取关键点预测结果
                le=(i.x()+int(plist[0]*i.w() - 10), i.y()+int(plist[1]*i.h())) # 计算左眼位置， 这里在w方向-10 用来补偿模型转换带来的精度损失
                re=(i.x()+int(plist[2]*i.w()), i.y()+int(plist[3]*i.h())) # 计算右眼位置
                nose=(i.x()+int(plist[4]*i.w()), i.y()+int(plist[5]*i.h())) #计算鼻子位置
                lm=(i.x()+int(plist[6]*i.w()), i.y()+int(plist[7]*i.h())) #计算左嘴角位置
                rm=(i.x()+int(plist[8]*i.w()), i.y()+int(plist[9]*i.h())) #右嘴角位置
                #a = img.draw_circle(le[0], le[1], 4)
                #a = img.draw_circle(re[0], re[1], 4)
                #a = img.draw_circle(nose[0], nose[1], 4)
                #a = img.draw_circle(lm[0], lm[1], 4)
                #a = img.draw_circle(rm[0], rm[1], 4) # 在相应位置处画小圆圈
                # align face to standard position
                src_point = [le, re, nose, lm, rm] # 图片中 5 坐标的位置
                T=image.get_affine_transform(src_point, dst_point) # 根据获得的5点坐标与标准正脸坐标获取仿射变换矩阵
                a=image.warp_affine_ai(img, img_face, T) #对原始图片人脸图片进行仿射变换，变换为正脸图像
                a=img_face.ai_to_pix() # 将正脸图像转为kpu格式
                #a = img.draw_image(img_face, (128,0)) #选择不显示仿射变换后的图像
                del(face_cut_128) # 释放裁剪人脸部分图片
                # calculate face feature vector
                fmap = kpu.forward(task_fe, img_face) # 计算正脸图片的196维特征值
                feature=kpu.face_encode(fmap[:]) #获取计算结果
                reg_flag = False
                scores = [] # 存储特征比对分数
                for j in range(len(record_ftrs)): #迭代已存特征值
                    score = kpu.face_compare(record_ftrs[j], feature) #计算当前人脸特征值与已存特征值的分数
                    scores.append(score) #添加分数总表
                max_score = 0
                index = 0
                for k in range(len(scores)): #迭代所有比对分数，找到最大分数和索引值
                    if max_score < scores[k]:
                        max_score = scores[k]
                        index = k
                if max_score > 85: # 如果最大分数大于85， 可以被认定为同一个人
                    a = img.draw_string(i.x(),i.y(), ("%s :%2.1f" % (names[index], max_score)), color=(0,255,0),scale=2) # 显示人名 与 分数
                else:
                    a = img.draw_string(i.x(),i.y(), ("Illegal Break in! :%2.1f" % (max_score)), color=(255,0,0),scale=2) #显示未知 与 分数
                    VECTOR += 1
                if key_pressed == 1: #如果检测到按键
                    key_pressed = 0 #重置按键状态
                    record_ftr = feature
                    record_ftrs.append(record_ftr) #将当前特征添加到已知特征列表
                    print("length:",len(record_ftrs),'\n')
                    print(record_ftr)
                break

        #fps =clock.fps() #计算帧率
        #print("%2.1f fps"%fps) #打印帧率
        a = lcd.display(img) #刷屏显示
        #kpu.memtest()
        key.disirq()

    else:
        #///口罩识别///#
        clock.tick()
        img = sensor.snapshot()
        code = kpu.run_yolo2(task, img)
        if code:
            faceVector += 1
            totalRes = len(code)

            for item in code:
                confidence = float(item.value())
                itemROL = item.rect()
                classID = int(item.classid())

                if confidence < 0.52:

                    _ = img.draw_rectangle(itemROL, color=color_B, tickness=5)
                    VECTOR += 1

                    continue

                if classID == 1 and confidence > 0.75:
                    _ = img.draw_rectangle(itemROL, color_G, tickness=5)
                    if totalRes == 1:
                        drawConfidenceText(img, (0, 0), 1, confidence)
                else:
                    _ = img.draw_rectangle(itemROL, color=color_R, tickness=5)
                    VECTOR += 1
                    if totalRes == 1:
                        drawConfidenceText(img, (0, 0), 0, confidence)

        _ = lcd.display(img)
        key.disirq()




