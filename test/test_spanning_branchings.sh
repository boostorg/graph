if [ $# != 1 ]
then
  echo " Usage: $0 n"
  echo "   n = number of vertices in complete graph"
  exit
fi

# Compile codes

g++ -I../include/ -o test_int_ranking test_int_ranking.cpp

g++ -I../include/ -o test_real_ranking test_real_ranking.cpp

# Test integer case

for i in {1..100}
do
  test_int_ranking $1
  if [ $? -eq 0 ]; then
    printf "."
  else
    echo "FAILED"
    exit
  fi
  sleep 1  # to ensure that random number generator creates new seed
done
echo ""
echo ""
echo "PASSED"
echo ""

# Test real case

for i in {1..100}
do
  test_real_ranking $1
  if [ $? -eq 0 ]; then
    printf "."
  else
    echo "FAILED"
    exit
  fi
  sleep 1  # to ensure that random number generator creates new seed
done
echo ""
echo ""
echo "PASSED"
echo ""

rm test_int_ranking test_real_ranking
