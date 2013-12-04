#! /bin/sh -e

DAEMON=/usr/local/bin/distrib #ligne de commande du programme
DAEMONUSER=root #utilisateur du programme
DAEMON_NAME=distrib #Nom du programme (doit être identique à l'exécutable)
DAEMON_CTRL_FILE=/proc/distrib.ctrl

PATH="/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin"

test -x $DAEMON || exit 0

. /lib/lsb/init-functions

d_start () {
        echo "Starting system $DAEMON_NAME Daemon"
        start-stop-daemon --background --name $DAEMON_NAME --start --chuid $DAEMONUSER --exec $DAEMON --DAEMON_CTRL_FILE
}

d_stop () {
        echo "Stopping system $DAEMON_NAME Daemon"
        start-stop-daemon --name $DAEMON_NAME --stop --retry 5 --signal INT
}

case "$1" in

        start|stop)
                d_${1}
                ;;

        restart|reload|force-reload)
                        d_stop
                        d_start
                ;;

        force-stop)
               d_stop
                killall -q $DAEMON_NAME || true
                sleep 2
                killall -q -9 $DAEMON_NAME || true
                ;;

        status)
                status_of_proc "$DAEMON_NAME" "$DAEMON" "system-wide $DAEMON_NAME" && exit 0 || exit $?
                ;;
        *)
                echo "Usage: /etc/init.d/$DAEMON_NAME {start|stop|force-stop|restart|reload|force-reload|status}"
                exit 1
                ;;
esac
exit 0
