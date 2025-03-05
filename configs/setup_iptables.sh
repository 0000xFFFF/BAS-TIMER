
# route port 5000 to 80
sudo iptables -t nat -A PREROUTING -p tcp --dport 5000 -j REDIRECT --to-port 80

sudo netfilter-persistent save
sudo netfilter-persistent reload
