
use strict "var";

my $i = 0;
my $F;
my @regs;
for $F (@ARGV)
{
    open F, "<$F" or next;
    my $T = join ("",<F>); close F;
    
    $T =~ s/\&/\&amp\;/sg;
    $T =~ s/¬/\&#AC\;/sg;
    $T =~ s/\*\//¬/sg;
    
    while ( $T =~
        s/ \/\*\*\s ([^¬]+) ¬ ([^{}]+) { /
            push_regs($F,$1,$2) /gsex
    ) {}
}

sub push_regs
{
 my ($filename, $comment, $prototype) = @_;
 
    $comment =~ s/¬/\*\//;
    my $info = { filename => $filename, comment => $comment, prototype => $prototype };
    push @regs, $info;
    
    return $prototype;
}
 
my $x;
my $V = "";
my $H = "";
my %names;
for $x (@regs)
{
    my $name = $$x{prototype};
    $name =~ s/^.*\b(\w+)\b\s*\(.*$/$1/s;
    my $indexproto = $$x{prototype};
    $indexproto =~ s/^(.*) \b(\w+)\b (\s*\(.*) $/
        "<tr><td><code>$1<\/code><\/td>"
       ."<td><a href=\"#$name\"><code>$2<\/code><\/a><\/td>"
       ."<td><code>$3<\/code><\/td><\/tr>" /sex;
    $H .= $indexproto;
    $names{$name} = $indexproto;
    $$x{prototype} =~ s/(^.*) (\b\w+\b) (\s*\(.*) $/
        "<code>$1<\/code>"
       ."<br \/><b><code>$2<\/code><\/b>&nbsp; <code>$3<\/code>" /sex;
    $V .=  "\n<dt><a name=\"$name\" /><code>".$$x{prototype}."</code><dt>";
    $$x{comment} =~ s/ ^\s?\s?\s?[^\*\s] (.*) $/ <code> $1 <\/code> /mgs;
    $$x{comment} =~ s/ ^\s?\s?\s?\* (.*) $/ <br \/> $1 /mgx;
    $$x{comment} =~ s/<c>/<code>/gs;
    $$x{comment} =~ s/<\/c>/<\/code>/gs;
    $V .= "\n<dd>".$$x{comment};
    $V .= "\n<p align=\"right\"><small>(".$$x{filename}.")</small></p></dd>";
}

$V =~ s/ \=\>(\s?)\b(\w+)\b / $1.&a_name($2) /gsex;
sub a_name 
{ 
    my $n = $_[0]; 
    if (exists $names{$n}) { return "<a href=\"#$n\"><code>$n</code></a>"; }
    else { return "<code>$n</code>"; }
}
$V =~ s/ \-\> /<small>-\&gt\;<\/small>/gsx;

open F, ">zziplib.html" or die "could notopen zziplib.html $!";
print F "<html><head><title> zziplib autodoc documentation </title></head>\n<body>\n";
print F "\n<h1>zziplib <small><small><i>-";
print F `grep -i "^version *:" zziplib.spec | sed -e "s/[Vv]ersion *: *//"`;
print F "</i></small></small></h1>";
print F "\n<table border=0 cellspacing=0 cellpadding=0>";
print F $H;
print F "\n</table>";
print F "\n<h3>Documentation</h3>\n";
print F "\n<dl>";
print F $V;
print F "\n</dl>";
print F "\n</body></html>\n";
close F;

