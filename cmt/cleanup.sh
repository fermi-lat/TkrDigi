tempfile=/tmp/setup$$
${CMTROOT}/mgr/cmt -quiet cleanup -sh -pack=GlastDigi -version=v0 -path=I:/packages/GaudiStuff $* >$tempfile; . $tempfile; /bin/rm -f $tempfile

