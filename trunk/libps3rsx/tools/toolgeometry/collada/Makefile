CPPFLAGS=-Wall -O3 -g -I/usr/include/libxml2

SOURCE= colladatool.cpp ColladaNode.cpp ColladaSource.cpp ColladaTriangles.cpp ColladaVertices.cpp ColladaAnimation.cpp ColladaMaterial.cpp ColladaBindMaterial.cpp ColladaController.cpp ColladaEffect.cpp ColladaFloatArray.cpp ColladaGeometry.cpp ColladaImage.cpp ColladaIDREFArray.cpp


OBJ = $(SOURCE:%.cpp=%.o)


all:collada

objc: $(SOURCE)
	$(CXX) $(CPPFLAGS) $(SOURCE) -c 


collada: $(OBJ) 
	$(CXX) -lxml2 $(CPPFLAGS) $(OBJ) -o collada


clean:
	$(RM) *.o *~ collada
