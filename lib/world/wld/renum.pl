#!/usr/bin/perl

sub do_wld {
  local ($corrected);

  open(OUT, ">$base.wld.out") || die ("can't open $base.wld.out: $!");
  open(FILE, "$base.wld") || die ("can't open $base.wld: $!");
    while (<FILE>) {
      if (/^#(\d+)/) {
        $begin = $1; last;
      }
    }
    print "$base.wld seems to begin at index $begin.\n\n";
    print "Where would you like to begin instead?\n";
    $answer = <STDIN>; $offset = $answer - $begin;
  close(FILE);

  open(FILE, "$base.wld") || die ("can't open $base.wld: $!");
    while (<FILE>) {
      if (/^#(\d+)\n/) {
        if ($1 < 99999) {
          $corrected = $1 + $offset;
          print OUT "#$corrected\n";
        } else {
          print OUT "#$1\n";
        }
      } elsif (/^([0|1|2|3|4])\W+(\d+)\W+(\d+)/) {
        $corrected = $3 + $offset;
        print OUT "$1 $2 $corrected\n";
      } else {
        print OUT;
      }
    }
  close(FILE);
  print "Done processing $base.wld\n\n";
}




sub do_obj {
  local ($corrected);

  open(OUT, ">$base.obj.out") || die ("can't open $base.obj.out: $!");

  open(FILE, "$base.obj") || die ("can't open $base.obj: $!");
    while (<FILE>) {
      if (/^#(\d+)/) {
        $begin = $1; last;
      }
    }
    print "$base.obj seems to begin at index $begin.\n\n";
    print "Where would you like to begin instead?\n";
    $answer = <STDIN>; $offset = $answer - $begin;
  close(FILE);

  open(FILE, "$base.obj") || die ("can't open $base.obj: $!");
    while (<FILE>) {
      if (/^#(\d+)\n/) {
        if ($1 < 99999) {
          $corrected = $1 + $offset;
          print OUT "#$corrected\n";
        } else {
          print OUT "#$1\n";
        }
      } else {
        print OUT;
      }
    }
  close(FILE);
  print "Done processing $base.obj\n\n";
}

sub do_mob {
  local ($corrected);

  open(OUT, ">$base.mob.out") || die ("can't open $base.mob.out: $!");
  open(FILE, "$base.mob") || die ("can't open $base.mob: $!");
    while (<FILE>) {
      if (/^#(\d+)/) {
        $begin = $1; last;
      }
    }
    print "$base.mob seems to begin at index $begin.\n\n";
    print "Where would you like to begin instead?\n";
    $answer = <STDIN>; $offset = $answer - $begin;
  close(FILE);

  open(FILE, "$base.mob") || die ("can't open $base.mob: $!");
    while (<FILE>) {
      if (/^#(\d+)\n/) {
        if ($1 < 99999) {
          $corrected = $1 + $offset;
          print OUT "#$corrected\n";
        } else {
          print OUT "#$1\n";
        }
      } else {
        print OUT;
      }
    }
  close(FILE);
  print "Done processing $base.mob\n\n";
}

print <<END
What basename would you like to convert?

i.e. if you want to convert tower40.wld, tower40.mob, and tower40.obj
     you should answer "tower40" (w/o quotes)

END
;

$base = <STDIN>;
chop($base);

&do_wld;
&do_obj;
&do_mob;
