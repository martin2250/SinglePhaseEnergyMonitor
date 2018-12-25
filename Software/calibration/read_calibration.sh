watch -n 1 "(date +%s | tr -d '\n' && printf '\\t' && curl -s http://singlephasemeter.fritz.box/metrics | sed -n -e '3p;6p' | rev | cut -d= -f1 | rev |tr '\n' '\t' && printf '\n') | tee -a logfile"
