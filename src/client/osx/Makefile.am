
EXTRA_DIST += \
		src/client/osx/Info.plist src/client/osx/PkgInfo \
		src/client/osx/English.lproj/main.nib/classes.nib \
		src/client/osx/English.lproj/main.nib/info.nib \
		src/client/osx/English.lproj/main.nib/objects.xib
#		src/client/osx/MAngclient.icns

noinst_PROGRAMS = ${CLIENT_BUNDLE}


mangclient.app: mangclient
	@mkdir mangclient.app
	@mkdir mangclient.app/Contents
	@mkdir mangclient.app/Contents/MacOS
	@mkdir mangclient.app/Contents/Resources
	@mkdir mangclient.app/Contents/Resources/English.lproj
	@mkdir mangclient.app/Contents/Resources/English.lproj/main.nib
	@mkdir mangclient.app/Contents/Resources/lib
	@mkdir mangclient.app/Contents/Resources/lib/data
	@mkdir mangclient.app/Contents/Resources/lib/edit
	@mkdir mangclient.app/Contents/Resources/lib/file
	@mkdir mangclient.app/Contents/Resources/lib/help
	@mkdir mangclient.app/Contents/Resources/lib/save
	@mkdir mangclient.app/Contents/Resources/lib/text
	@mkdir mangclient.app/Contents/Resources/lib/user
	@mkdir mangclient.app/Contents/Resources/lib/pref
	@mkdir mangclient.app/Contents/Resources/lib/xtra
	@mkdir mangclient.app/Contents/Resources/lib/xtra/font
	@mkdir mangclient.app/Contents/Resources/lib/xtra/graf
	@mkdir mangclient.app/Contents/Resources/lib/xtra/Sound
	@cp mangclient mangclient.app/Contents/MacOS
	@$(eval DYLIBS=$(shell otool -L mangclient | grep SDL | awk -F' ' '{ print $$1 }'))
	@for dylib in ${DYLIBS} ; do \
		NAME=`basename $$dylib` ; \
		echo "dylib: $$dylib -> $$NAME" ; \
		cp $$dylib mangclient.app/Contents/MacOS/ ; \
		install_name_tool -change $$dylib @executable_path/$$NAME mangclient.app/Contents/MacOS/mangclient ; \
	done
	@cp src/client/osx/launch_mangclient.sh mangclient.app/Contents/MacOS
	@cp src/client/osx/mangclient.icns mangclient.app/Contents/Resources
	@cp src/client/osx/Info.plist mangclient.app/Contents
	@cp src/client/osx/PkgInfo mangclient.app/Contents
	@cp src/client/osx/English.lproj/main.nib/*ib mangclient.app/Contents/Resources/English.lproj/main.nib
	@cp lib/edit/*txt mangclient.app/Contents/Resources/lib/edit
	@cp lib/help/*txt mangclient.app/Contents/Resources/lib/help
	@cp lib/help/*hlp mangclient.app/Contents/Resources/lib/help
	@cp lib/save/server* mangclient.app/Contents/Resources/lib/save
	@cp lib/pref/*prf mangclient.app/Contents/Resources/lib/pref
	@cp lib/user/*prf mangclient.app/Contents/Resources/lib/user
	@cp lib/xtra/font/*FON mangclient.app/Contents/Resources/lib/xtra/font
	@cp lib/xtra/font/*hex mangclient.app/Contents/Resources/lib/xtra/font
	@cp lib/xtra/graf/*png mangclient.app/Contents/Resources/lib/xtra/graf
	@cp lib/xtra/sound/*wav mangclient.app/Contents/Resources/lib/xtra/sound
	@cp lib/xtra/sound/sound.cfg mangclient.app/Contents/Resources/lib/xtra/sound

clean-local:
	-rm -rf mangclient.app
