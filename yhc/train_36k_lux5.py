# -*- coding: utf-8 -*-
"""
Created on Tue Apr  2 14:04:08 2019

@author: user
"""
import random
import torch
import torch.nn as nn
import csv
import pandas as pd
import numpy as np
import torch.utils.data as data
from torch.autograd import Variable


class FaceLandmarksDataset(data.Dataset):
	"""Face Landmarks dataset."""
	def __init__(self, csv_file,symbol,upsample):
		"""
        Args:
            csv_file (string): Path to the csv file with annotations.
            root_dir (string): Directory with all the images.
            transform (callable, optional): Optional transform to be applied
                on a sample.
        """
		self.landmarks_frame = pd.read_csv(csv_file)
		self.symbol = symbol
		self.upsample = upsample
	def __len__(self):
		#print len(self.landmarks_frame)
		return len(self.landmarks_frame)
		#return 6
	def __getitem__(self, idx):  

		landmarks = self.landmarks_frame.iloc[idx,:].as_matrix() 
		ind = np.reshape(landmarks,(-1,self.symbol*self.upsample*3+1))     ####   
		label_ind = ind[0,0] 
		label_ind = int(label_ind)   
		label = label_ind
#		label = np.zeros([1,8])
#		label[0,label_ind] =1
#		label = np.reshape(label,(8))    
#		label = torch.from_numpy(label)
        
		BRGBR = ind[0,1:]
		BRGBR = np.reshape(BRGBR, (self.symbol*self.upsample,3)) ###
		B_temp = BRGBR[:,2]
		B_temp = np.reshape(B_temp, (self.symbol*self.upsample,1))###
		R_temp = BRGBR[:,0]
		R_temp = np.reshape(R_temp, (self.symbol*self.upsample,1))###
		BRGBR = np.hstack((B_temp,BRGBR,R_temp))
		BRGBR = np.reshape(BRGBR,(-1,self.symbol*self.upsample,5))      ###  
		BRGBR = torch.from_numpy(BRGBR)
		sample = {'BRGBR':BRGBR,'label':label}
		return sample
    
class CNN(nn.Module):
    def __init__(self,symbol,upsample):
        super(CNN, self).__init__()
        self.conv1 = nn.Sequential(  # input shape (1, 27, 5)
            nn.Conv2d(
                in_channels=1,      # input height
                out_channels=5,    # n_filters
                kernel_size=3,      # filter size
                stride=1,           # filter movement/step
                padding=1,      # 如果想要 con2d 出来的图片长宽没有变化, padding=(kernel_size-1)/2 当 stride=1
            ),      # output shape (10, 25, 3)
            nn.ReLU(),    # activation
            nn.BatchNorm2d(5)
            # nn.MaxPool2d(kernel_size=2),    # 在 2x2 空间里向下采样, output shape (16, 14, 14)
        )
        self.conv2 = nn.Sequential(  # input shape (10, 25, 3)
            nn.Conv2d(
                in_channels=5, 
                out_channels=3, 
                kernel_size=(5,5), 
                stride=1, 
                padding=2
            ),  # output shape (5, 25, 3)
            nn.ReLU(),  # activation
            nn.BatchNorm2d(3)
            # nn.MaxPool2d(kernel_size=2),  # output shape (32, 7, 7)
        ) 
        self.out1 = nn.Sequential(
            nn.Linear(3 * (symbol*upsample) * 5, 8),
            nn.ReLU(),
#            nn.Linear(25, 8),
#            nn.ReLU()
        )

    def forward(self, x):
        x = self.conv1(x)
        x = self.conv2(x)
        #x = self.conv3(x)
        x = x.view(x.size(0), -1)   # 展平多维的卷积图成 (batch_size, 32 * 7 * 7)
        x = self.out1(x)
#        x = self.out2(x)
#        x = self.out3(x)
#        x = self.out4(x)
        #x = self.out2(x)
        #nn.ReLU()
        #output = self.out3(x)
        output = x
        return output

symbol = 5
upsample =3
sampleRate = 32

device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
# Initialize model
model = CNN(symbol,upsample)
model = model.to(device)



#criterion = torch.nn.MSELoss(reduction='sum')
criterion = torch.nn.CrossEntropyLoss()
lr_temp=1e-2
lr_temp_init = 1e-2
momentum_temp=0.7
optimizer = torch.optim.Adam(model.parameters(), lr=lr_temp, betas=(0.9, 0.999), eps=1e-08,)
#optimizer = torch.optim.SGD(model.parameters(), lr=lr_temp, weight_decay = 1e-5)    #, momentum=0.9



################################################## retrain
#filename = 'F:/programming/ML in OCC/Final test/'+str(sampleRate)+'k regular/modelb_'+str(symbol)+'_'+str(upsample)+'.pkl'	 
#model.load_state_dict(torch.load(filename))
#optimizer = torch.optim.SGD(model.parameters(), lr=1e-5, momentum=0.3)    #, momentum=0.9
 



filename = './class_train_5_3_36k_lux5.csv'
dataset = FaceLandmarksDataset(filename,symbol,upsample)
train_loader = torch.utils.data.DataLoader(dataset, batch_size=252, shuffle=True)
epochnum = 40
loss_b = 50
loss_save=[]
for i in range(epochnum):
    for t, data in enumerate(train_loader):
        #print (data)
        inputs,label = Variable(data['BRGBR']),Variable(data['label'])
        inputs = inputs.to(device, dtype=torch.float32)
        label = label.to(device,dtype=torch.long)
        

        outputs = model(inputs)
        loss = criterion(outputs,label)
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
        if t==0:    
            print(i, loss.item())
            loss_save.append(loss.item())
            if loss.item()<loss_b:
                loss_b = loss.item()
                filename = './modelb_36_lux5.pkl'	      
                torch.save(model.state_dict(), filename) 
                

    lr_temp = lr_temp_init * (0.1 ** (i // 10))
    if lr_temp < 1e-5:
        lr_temp = 1e-5    
#    if i==10:
#        momentum_temp = 0.5
#    if i==20:
#        momentum_temp = 0.3
#    if i==30:
#        momentum_temp = 0.1
    optimizer = torch.optim.Adam(model.parameters(), lr=lr_temp, betas=(0.9, 0.999), eps=1e-08,)    #, momentum=0.9  
        #optimizer = torch.optim.SGD(model.parameters(), lr=lr_temp,weight_decay = 1e-5)    #, momentum=0.9         
            
#    if i==30:
#        optimizer = torch.optim.SGD(model.parameters(), lr=1e-4, momentum=0.3)    #, momentum=0.9
#    if i==60:
#        optimizer = torch.optim.SGD(model.parameters(), lr=5e-5, momentum=0.3)    #, momentum=0.9
#    if i==90:
#        optimizer = torch.optim.SGD(model.parameters(), lr=1e-5, momentum=0.3)    #, momentum=0.9
    
filename = './modelb_36_lux5.pkl'	    
torch.save(model.state_dict(), filename)    

 	
p = pd.DataFrame(loss_save)
filename = './loss_save_36_lux5.csv'
p.to_csv(filename)        
		    

