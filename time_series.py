from imutils import contours 
from skimage import measure  
import numpy as np          
import argparse
import imutils
import cv2
import matplotlib.pyplot as plt

values = []

for i in range(100, 962):
    image_address = "./resources/image_" + str(i) + ".jpg"

    ## read the image
    image = cv2.imread(image_address)

    ## convert BGR to RGB, we won't use this in any of the processes, just to test image loading
    rgb_image = cv2.cvtColor(image , cv2.COLOR_BGR2RGB) 

    ## convert BGR to greyscale
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

    ## blur the image with a gussian filter to remove any noise, and also to soften the image
    blurred = cv2.GaussianBlur(gray, (11, 11), 0)

    # ## plot images in a nice array. We have to define the colormap for imshow to get a black and white image, as the default 
    # ## color map shows colors between yellow (pixel = 255) and black (pixel = 0)
    # fig , ax = plot_imshow_in_array([rgb_image , gray , blurred] , \
    #                                 max_image_in_row = 4 , \
    #                                 plots_kwargs_list = [{} , {'cmap':'gray', 'vmin':0, 'vmax':255} , {'cmap':'gray', 'vmin':0, 'vmax':255}] )

    # ## use this command to place enough spacing between the borders of images and the numbers
    # fig.tight_layout()

    thresh = cv2.threshold(blurred, 200 , 255 , cv2.THRESH_BINARY)[1]

    thresh = cv2.erode(thresh, None, iterations=2)
    thresh = cv2.dilate(thresh, None, iterations=4)

    MIN_ALLOWED_PIXEL_IN_REGION = 5000
    MAX_GREY_VALUE = 255

    ## find out regions and label them
    labels = measure.label(thresh, connectivity=2, background=0)


    ## The list holding all contiguous masks that will be found
    individual_masks = []

    ## loop over the unique components
    for label in np.unique(labels):
        # if this is the background (dark) label, ignore it
        if label == 0:
            continue
            
        # otherwise, construct the label mask and count the number of pixels 
        labelMask = np.zeros(thresh.shape, dtype="uint8")
        labelMask[labels == label] = MAX_GREY_VALUE
        numPixels = cv2.countNonZero(labelMask)

        
        # this condition filters "very" small regions
        if numPixels > MIN_ALLOWED_PIXEL_IN_REGION:
            #plt.figure()        
            individual_masks.append(labelMask)
    
    ## make a copy of the individual_mask to avoid them from being overwritten
    individual_masks_cp = [mask.copy() for mask in individual_masks]

    value = 0

    for idx , current_mask in enumerate(individual_masks_cp):
        cnts = cv2.findContours(current_mask.copy(), cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)
        cnts = imutils.grab_contours(cnts)
    #     cnts = contours.sort_contours(cnts)[0]

        ## create a mask to hold the values of the enclosing circle, we also fill inside the cirlce. 
        filled_circle = np.zeros(current_mask.shape)

        ## find the coordinates of the center and the radius of the circle
        ((cX, cY), radius) = cv2.minEnclosingCircle(cnts[0])
        
        ## fill the mask values surrounded by the circle
        cv2.circle(filled_circle, (int(cX), int(cY)), int(radius),(255, 255, 255), -1)
        
        ## count the number of pixels in the circle
        circle_area = cv2.countNonZero(filled_circle)
        
        ## count the number of non-zero pixels in the original region
        mask_non_zero_area = cv2.countNonZero(individual_masks[idx])
        
        ## plot the circle and the region, and show the filled ratio in each circle
        # plot_imshow_in_array([filled_circle , individual_masks[idx]] , max_image_in_row = 4)

        if ((mask_non_zero_area / circle_area) > 0.9):
            value = 1

        #plt.title("circles' filled ratio = {:.2}".format(mask_non_zero_area / circle_area))

    values.append(value)

# 0xAAAA 1010101010101010

def demodulate(data):
    result = 0
    for i in range(0, 16):
        if (i % 2 == 0):
            if (data[i:i+2] == 1):
                result = result | (1 << (i // 2))
        else:
            continue 
    return result

preamble = np.array([1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0])
found = False
start = -1
while ((not found) and (len(values) > 1)):
    for i in range(0, len(values)-16):
        if (np.array_equal(preamble, np.array(values[i:i+16]))):
            found = True
            start = i
        
            break
    if (not found):
        values2 = []
        a = False
        for x in range(0, len(values)-1):
            if ((x != (len(values)-1)) and (values[x] == values[x+1]) and (not a)):
                a = True
                continue
            values2.append(values[x])
            a = False
        # print("Original: ", values)
        values = values2
        # print("New: ", values2)

print(demodulate(values[start+16:start+32]))

plt.plot(values)
plt.show()