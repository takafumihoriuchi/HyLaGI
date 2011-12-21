package test.graph;

import java.awt.Menu;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import test.FrontEnd;

public class PopupMenu extends JPopupMenu
{
	private static final long serialVersionUID = 1L;

	JMenuItem menuitem;
	JMenu menu;
	private PhaseViewer _pv;

	public PopupMenu(PhaseViewer pv)
	{
		_pv = pv;

		menuitem = new JMenuItem("Clear");
		menuitem.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				FrontEnd.println("Clear");
			}
		});
		add(menuitem);

			JMenu menuhelp = new JMenu("help");
			menuitem = new JMenuItem("Controll Instructions");
			menuitem.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					FrontEnd.println("操作説明");
				}
			});
			menuhelp.add(menuitem);

			JMenu menug = new JMenu("Glossary");
			menug.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					FrontEnd.println("用語解説");
				}
			});
			menuhelp.add(menug);

				menuitem = new JMenuItem("いろいろなもの");
				menuitem.addActionListener(new ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						FrontEnd.println("いろいろなもの");
						_pv.consTextArea.println("いろいろどきゅん");
					}
				});
				menug.add(menuitem);

				menuitem = new JMenuItem(_pv.getHelpText());
				menuitem.addActionListener(new ActionListener()
				{
					public void actionPerformed(ActionEvent e)
					{
						FrontEnd.println("さまざまなもの");
						_pv.consTextArea.errPrint("どこにどーなんの");
					}
				});
				menug.add(menuitem);


			add(menuhelp);


	}
}
