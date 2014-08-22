
all:
	@make -f makeparticle PROJECTNAME=particle-mt BUILD=build-mt
	@make -f makeparticle PROJECTNAME=particle-st BUILD=build-st CFLAGS_EX=-D_SINGLETHREAD
	
clean:
	@make clean -f makeparticle PROJECTNAME=particle-mt BUILD=build-mt
	@make clean -f makeparticle PROJECTNAME=particle-st BUILD=build-st