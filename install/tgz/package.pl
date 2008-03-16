#!/usr/bin/perl -w

# Step 1
# Obtain version

$config_h = "../../src/common/defines.h";

open(DAT, $config_h) || die("config.h not found");
@raw_data=<DAT>;
close(DAT);

foreach $line (@raw_data) {
    chop ($line);
    if ($line =~ /VERSION_(MAJOR|MINOR|PATCH)\s+(\d+)/) {
	$ver{$1} = $2;
    }
}

#$ver_u = $ver{MAJOR}."_".$ver{MINOR}."_".$ver{PATCH};
$ver_d = $ver{MAJOR}.".".$ver{MINOR}.".".$ver{PATCH};
#$ver_h = $ver{MAJOR}."-".$ver{MINOR}."-".$ver{PATCH};

# Step 2
# Prepare directory structure

$newdir = "mangband-".$ver_d;

system "rm -rf ./$newdir";

mkdir $newdir;
mkdir $newdir."/src";
mkdir $newdir."/lib";
mkdir $newdir."/tmp";


system "cp ../../* $newdir";
#system "rm -rf $newdir/install";
#system "rm -rf $newdir/tmp";
system "cp -R ../../src $newdir/";
system "cp -R ../../lib $newdir/";

# Step 3
# Tar it!

system "rm -f $newdir.tar.gz";
system "tar -cvzf $newdir.tar.gz --exclude='.svn' --exclude='*.o' $newdir >/dev/null";

# Step 4
# Help .deb script

#system "cp $newdir.tar.gz ../deb/";
#system "cp ../deb/$newdir.tar.gz ../deb/mangband_".$ver_d.".orig.tar.gz";
system "echo $ver_d >./version";

# Step 5
# Display funny message

print "\n";
print "Mangband $ver_d > $newdir.tar.gz \n";
print "\n";