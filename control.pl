#!/usr/local/bin/perl
use strict;
use Time::HiRes qw(usleep nanosleep gettimeofday);

if($ARGV[0] eq 'config'){
    open(FH,"<",$ARGV[1]);
    while(<FH>){
        chomp;
        my @a=split(/\s+/,$_);
        config(@a);
    }
    close(FH);
}else{
    config(@ARGV);
}

exit;
sub wsend {
    my ($cmd,$a1,$a2,$a3,$a4) = @_;
    my $cs=255&($cmd+$a1+$a2+$a3+$a4);
    my $crc=256-$cs;

    printf STDERR "%02x %02x %02x %02x %02x\n",$cmd,$a1,$a2,$a3,$a4,$crc;
    printf "%08b%08b%08b%08b%08b%08b\n",$cmd,$a1,$a2,$a3,$a4,$crc;

}
sub config  {
    my @A=@_;
    if($A[0] eq 'set'){
        my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);

        wsend(0x81,$wday,$hour,$min,$sec);
    }elsif($A[0] eq 'ring'){
        wsend(0x82,0,0,0,$A[1]);
    }elsif($A[0] eq 'setring'){
        for(2..9){
            $A[$_]=0 unless($A[$_]);
        }
        wsend(0x20+$A[1],$A[2],$A[3],$A[4],$A[5]);
        nanosleep(100000000);
        wsend(0x30+$A[1],$A[6],$A[7],$A[8],$A[9]);
    } elsif($A[0] eq 'alarm'){
        #control.pl alarm 0 dow h m pattern
        wsend(0x83,($A[1] << 4) | ($A[5]&0x0f),$A[2],$A[3],$A[4]);
    }
    nanosleep(100000000);
}


