Import('*')

targets = [
	env.Program('eclipse', Glob('*.cpp'))
]

for target in targets:
	env.Depends(target, imgs)

Return('targets')
