CXX      = g++
CXXFLAGS = -O2 -std=c++17 -Wall -Wextra

all: asn1_a asn1_b

asn1_a: asn1.cc
	$(CXX) $(CXXFLAGS) -DBUILD_ASN1_A asn1.cc -o asn1_a

asn1_b: asn1.cc
	$(CXX) $(CXXFLAGS) -DBUILD_ASN1_B asn1.cc -o asn1_b

clean:
	rm -f *.o asn1_a asn1_b