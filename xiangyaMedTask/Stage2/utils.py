# -*- coding: utf-8 -*-
"""
Created on Sat Aug 21 10:31:38 2021
@author: crisprhhx
"""

import os
import pandas as pd
import numpy as np
import tensorflow as tf
import keras
from keras import backend as K
from skimage import io
from keras.applications.vgg16 import VGG16
import matplotlib.pyplot as plt
from keras import Sequential
from keras.applications.vgg16 import VGG16
from keras.models import Model
from keras.preprocessing import image
from keras.preprocessing.image import ImageDataGenerator
from keras.layers import Dense,GlobalAveragePooling2D

def get_vgg16():

    base_model = VGG16(weights=None,include_top=False,input_shape=(256,256,4))
    globpool_layer = GlobalAveragePooling2D()
    globpool_layer_tensor = globpool_layer(base_model.output)
    densehidden_layer = Dense(256,activation='relu',name = 'featurelayer')
    densehidden_layer_tensor = densehidden_layer(globpool_layer_tensor)
    denseout_layer = Dense(2,activation='sigmoid')
    denseout_layer_tensor = denseout_layer(densehidden_layer_tensor)
    
    model = Model(inputs=base_model.input,outputs=denseout_layer_tensor)
    
    for layer in model.layers:
        layer.trainable = False
    model.layers[-1].trainable = True
    model.layers[-2].trainable = True
    model.layers[-3].trainable = True
    
    return model

def rescale(x, Max=1, Min=0):
    x_std = (x-x.min())/(x.max() - x.min())
    x_scaled = x_std * (Max - Min) + Min
    return x_scaled

def data_PathWrapper(image_data_root, mask_data_root):
    # returns a dataframe of patient's mri&mask and corresponding label.
    
    data_map = []
    for sub_dir in os.listdir(image_data_root):
        try:
            label = sub_dir
            sub_dir_path = image_data_root + sub_dir
            for patiendfolder in os.listdir(sub_dir_path):
                for imagefile in os.listdir(sub_dir_path+'/'+patiendfolder+'/'):
                    image_path = sub_dir_path+'/'+patiendfolder+'/'+imagefile
                    mask_path = mask_data_root+label+'/'+patiendfolder+'/'+imagefile[:-4]+'_mask.txt'
                    data_map.extend([patiendfolder, image_path, mask_path, label])
        except Exception as e:
            print(e)
            
    
    
    
    #b = a[i:j:s],这里的s表示步进.很奇怪，可能是因为dataframe的输入是字典，而字典需要通过这样的切片方式来操作吧
    df = pd.DataFrame({"patient_id":data_map[::4],
                      "image_path":data_map[1::4],
                      "mask_path":data_map[2::4],
                      "label":data_map[3::4]})
    return df


class DataGenerator(tf.keras.utils.Sequence):
    def __init__(self, img_path_list, mask_path_list, label_list, batch_size = 10, img_h = 256, img_w = 256, shuffle = True):
        
        self.img_path_list = img_path_list
        self.mask_path_list = mask_path_list
        self.label_list = label_list
        self.batch_size = batch_size
        self.img_h = img_h
        self.img_w = img_w
        self.shuffle = shuffle
        self.on_epoch_end()
    
    def __len__(self):
        'Get the number of batches per epoch'

        return int(np.floor(len(self.img_path_list)) / self.batch_size) #it should be 4.
    
    def on_epoch_end(self):
        'Used for updating the indices after each epoch, once at the beginning as well as at the end of each epoch'
        #getting the array of indices based on the input dataframe
        self.indexes = np.arange(len(self.img_path_list))
        
        if self.shuffle:
            np.random.shuffle(self.indexes)

    
    def __getitem__(self, index):
        '''
        凡是在类中定义了这个__getitem__ 方法，那么它的实例对象（假定为p），可以像这样:
        p[key] 取值，当实例对象做p[key] 运算时，会调用类中的方法__getitem__。
        一般如果想使用索引访问元素时，就可以在类中定义这个方法: __getitem__(self, key)
        https://blog.csdn.net/chituozha5528/article/details/78354833
        '''
        #generate index of batch_size length
        indexes = self.indexes[index* self.batch_size : (index+1) * self.batch_size]
        
        #get the Image path corresponding to the indexes created above based on batch size
        list_imgs = [self.img_path_list[i] for i in indexes]
    
        #get the Mask path corresponding to the indexes created above based on batch size
        list_masks = [self.mask_path_list[i] for i in indexes]
        
        #get the corrsponding labels
        list_labels = [self.label_list[i] for i in indexes]
        
        #generate data for X and y
        X, y = self.__data_generation(list_imgs, list_masks, list_labels)
        
        return X, y
    
    def __data_generation(self, list_imgs, list_masks, list_labels):
        "generate the data corresponding the indexes in a given batch of images"
        
        # create empty arrays of shape (batch_size,height,width,depth) 
        #Depth is 4 for input(3 for the RGB channels and 1 for the mask concatenated) 
        #and depth is taken as 1 for output becasue mask consist only of 1 channel.
        X = np.empty((self.batch_size, self.img_h, self.img_w, 4))
        y = []
        
        #iterate through the data rows
        for i in range(len(list_imgs)):
            #get the corresponding path
            img_path = list_imgs[i]
            mask_path = list_masks[i]
            
            #get raw data
            img = io.imread(img_path)
            mask = np.loadtxt(mask_path)
            label = float(list_labels[i])
            
            #convert img to numpy array of type float64
            img = np.array(img, dtype = np.float64)
            img = rescale(img)
            mask = np.expand_dims(mask, axis = -1)
            #>>>>>DEBUG>>>>>
            #打印一下img array的值的分布是不是0-255
            #打印一下mask array的分布是不是0-255
            #concatenate之后的结果，要不要归一化
            #VGG16的输入是[0,1]还是[-1,1]，有没有其他要求
            
            #standardising 
            img -= img.mean()
            img /= img.std()
            mask -= mask.mean()
            mask /= mask.std()
            
            #concatenate the image and mask
            data_concat = np.concatenate((img, mask), axis = -1)
            
            #<<<<<DEBUG<<<<
            
            X[i,] = data_concat
            y.append(label)
        
        y = np.array(y)
        y = tf.one_hot(y, depth=2)
        return X, y














