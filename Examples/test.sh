echo "Starting tests..."
echo "--------------------------------------------------------------"
echo "AnyTest.dino:"
dino AnyTest.dino -o test
./test
echo "--------------------------------------------------------------"

echo "MultRetTest.dino:"
dino MultRetTest.dino -o test
./test
echo "--------------------------------------------------------------"

echo "BrainF.dino:"
dino BrainF.dino -o test
./test
echo "--------------------------------------------------------------"

echo "LLTest.dino:"
dino LLTest.dino -o test
./test
echo "--------------------------------------------------------------"

echo "Vector.dino:"
dino Vector.dino -o test
./test
echo "--------------------------------------------------------------"

echo "Map.dino:"
dino Map.dino -o test
./test
echo "--------------------------------------------------------------"