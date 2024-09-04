#pragma once
#include "Instruction.h"
#include "../Image/Image.h"
#include "../GUI/ImageEditScene.h"

// Allows to load CW2 bytecode and execute it
class CW2VM final : public QObject {
	Q_OBJECT

private:
	QList<Instruction> m_byteCode;
	QList<float> m_stack;
	QList<float> m_localVarStack;
	QList<float> m_globalVars;
	QList<uint32_t> m_callStack;

	QList<Image*> m_imageBuffers;
	ImageEditScene* m_pScene;

	Image* m_image;

	Color m_color = Color::Black;
	float m_toolWidth = 3;

	uint32_t m_width;
	uint32_t m_height;

	uint32_t m_pos = 0;

	bool m_isLoaded = false; // whether the vm has some code loaded
	bool m_shallHalt = false;

public:
	CW2VM(ImageEditScene* parent);

	~CW2VM();

	utils::Result<Void> loadFromFile(const QString& fileName);

	// So as to stop it forcefully even if the "sleep" is uxecuting
	void forceHalt();

public slots:
	void execute();

private:
	void delayedExecute();
	void resetVMState();
};