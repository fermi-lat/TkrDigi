set tempfile=/tmp/setup$$
${CMTROOT}/mgr/cmt -quiet cleanup -csh -pack=GlastDigi -version=v0 -path=I:/packages/GaudiStuff $* >$tempfile; source $tempfile; /bin/rm -f $tempfile

