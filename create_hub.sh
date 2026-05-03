APP_NAME="tap-hub"
PORTS_COUNT=${1:-2}

g++ hub.cpp -o "$APP_NAME"

args=()

for i in $(seq 1 "$PORTS_COUNT"); do
    args+=("hub_tap$i");
done

for i in  ${args[@]}; do
    sudo ip tuntap add dev "$i" mode tap
    sudo ip link set "$i" up;
done

sudo "./$APP_NAME" ${args[@]}
sleep 1
ip -br a | grep hub_tap

for i in  ${args[@]}; do
    sudo ip link set "$i" down
    sudo ip tuntap del dev "$i" mode tap;
done

ip -br a | grep hub_tap

