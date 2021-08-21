# -*- coding: utf-8 -*-
"""
Created on Sun Aug 21 22:02:44 2021
@author: HuangHongxiang
"""
import os
import numpy as np
import tensorflow as tf
import keras
from keras import backend as K
from keras import layers,activations
import matplotlib.pyplot as plt
import matplotlib.cm as cm
from utils import get_vgg16, data_PathWrapper, DataGenerator

image_data_root = "DATASET/xiangya_selected_10/"
mask_data_root = "DATASET/xiangya_selected_10_mask/"

# ========================================
# define pretrained vgg16 model
model = get_vgg16()
model.summary()
model.compile(optimizer='rmsprop',loss='binary_crossentropy',metrics=['acc'])

# ========================================
# prepare our dataset
df = data_PathWrapper(image_data_root, mask_data_root)

img_path_list = df["image_path"].values
mask_path_list = df["mask_path"].values
label_list = df["label"].values

train_data = DataGenerator(img_path_list, mask_path_list, label_list)

#Training.
history = model.fit(train_data, 
                  epochs = 70
                )























'''
TRAINING_DIR = 'train/'
train_datagen=ImageDataGenerator(rescale=1.0/255.)
train_generator = train_datagen.flow_from_directory(TRAINING_DIR,
													batch_size=100,
													class_mode="categorical",
													target_size=(32,32))
VALIDATION_DIR = 'test/'
validation_datagen = ImageDataGenerator(rescale=1.0/255.)
validation_generator = validation_datagen.flow_from_directory(VALIDATION_DIR,
														  batch_size=100,
															  class_mode="categorical",
															  target_size=(32,32))
															  
print('train_generator:\n',train_generator.class_indices,'validation_generator\n',validation_generator.class_indices)
history = model.fit_generator(train_generator,epochs=2,verbose=1,validation_data=validation_generator)
'''
#======save model======
'''
model.save('I_love_coursera!.h5')
#==or==
json_string = model.to_json()
open("ILoveKeras!.json",'w').write(json_string)
model.save_weights('weights.h5')
'''
#======================

#visualization
'''

acc=history.history['acc']
val_acc=history.history['val_acc']
loss=history.history['loss']
val_loss=history.history['val_loss']
epochs=range(len(acc)) 
plt.plot(epochs, acc, 'r', "Training Accuracy")
plt.plot(epochs, val_acc, 'b', "Validation Accuracy")
plt.title('Training and validation accuracy')
plt.figure()
plt.plot(epochs, loss, 'r', "Training Loss")
plt.plot(epochs, val_loss, 'b', "Validation Loss")
plt.figure()


img_path = "airplane.jpg"
img = image.load_img(img_path,target_size=(224,224))
x=image.img_to_array(img)
x=np.expand_dims(x,axis=0)
x=preprocess_input(x)
preds=model.predict(x)
print('Predicted:\n',decode_predictions(preds,top=3)[0])

'''






