stripe: 
	g++ -Wall -o stripe stripe.cpp -lfcgi -lfcgi++ -lcurl -std=gnu++11

clean:
	rm stripe

run:
	spawn-fcgi -p 9004 -n stripe
