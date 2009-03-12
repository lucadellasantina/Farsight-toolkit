#python script to register pairs specified in a file, which contains a pair of
#image names per line.

import os, sys

# the main function
def register(argv):
    if len(argv) < 2:
        print 'Arguments: image_dir pair_list'
        sys.exit(1)

    f=open(argv[1],'r');
    f_o = open(argv[1]+'.out','w')
    f_xforms = open('xxx_123.txt','w')
    names=[]
    xforms=[]

    #pairwise registration
    for line in f:
        s_line = line.rstrip().rstrip('\n')
        pos = s_line.find(' ')
        from_image = s_line[:pos]
        to_image = s_line[pos+1:]
            
        # perform registration
        success = os.system('register_pair.exe '+argv[0]+from_image+' '+ argv[0]+to_image +' -remove_2d');
        if success == 0:
            # Add the names to the list if not already there. The list
            # keeps potential images which can be the anchor iamge for
            # montaging
            found = False 
            for name in names:
                if name == from_image:
                    found = True
                    break
            if not found: names.append(from_image)
        
            found = False 
            for name in names:
                if name == to_image:
                    found = True
                    break
            if not found: names.append(to_image)
        
            # Add the transformed.xml to the list
            from_dot = from_image.find('.')
            to_dot = to_image.find('.');
            f_xforms.write(from_image[:from_dot]+"_to_"+to_image[:to_dot]+"_transform.xml\n")
        else:
            f_o.write("From "+from_image+" to "+to_image+" failed")
            print "From "+from_image+" to "+to_image+" failed"
            
    # Perform joint registration write the xform list to a
    # temporary file and remove it after joint registration
    f_xforms.close()
    f_o.close()
    print("\nSTART register_joint.exe...")
    os.system('register_joint.exe xxx_123.txt')
    print("DONE")
    #os.system('rm xxx_123.txt')    #not cross platform!!

    # perform montaging using the first image as the anchor
    print("\nSTART...")
    if(argv[0]==''):
        cmd = "mosaic_images.exe joint_transforms.xml " + names[0]
    else:
        cmd = "mosaic_images.exe joint_transforms.xml " + names[0] + " -path " + argv[0]
    os.system(cmd)
    print("DONE")

    # TEMP FILE CLEANUP:
    print("\nCLEANING TEMP FILES...")
    os.remove(argv[1]+'.out')
    os.remove('xxx_123.txt')
    print("DONE")

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print 'Usage: '+sys.argv[0]+' image_dir pair_list'
        sys.exit(1)
    register(sys.argv[1:])
