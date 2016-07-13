;==========================================
;			ZapperOS Common
;------------------------------------------
;Done By Omar Emad Eldin
;Credits To : Mike
;==========================================

%define PModeBase 		0x100000

%define FileBuffer		0x3000

%define MMapAddress		0x1000

ImageName DB 'KRNL1.SYS'
%strlen ImageNameLength 'KRNL1.SYS'
ImageSize dd 0