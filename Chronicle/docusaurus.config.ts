import type { Config } from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';

const config: Config = {
  title: 'Project Durian Chronicle',
  tagline: 'Project Durian 개발 위키와 개발일지',

  url: 'https://sunnymoon724.github.io',
  baseUrl: '/',

  organizationName: 'Sunnymoon724',
  projectName: 'project-durian',

  onBrokenLinks: 'throw',
  onBrokenMarkdownLinks: 'warn',

  i18n: {
    defaultLocale: 'ko',
    locales: ['ko', 'en'],
  },

  presets: [
    [
      'classic',
      {
        docs: {
          sidebarPath: './sidebars.ts',
          routeBasePath: 'docs',
        },
        blog: {
          showReadingTime: true,
          blogTitle: '개발일지',
        },
        theme: {
          customCss: './src/css/custom.css',
        },
      } satisfies Preset.Options,
    ],
  ],

  themeConfig: {
    navbar: {
      title: 'Project Durian',
      items: [
        {to: '/docs/intro', label: '개발 위키', position: 'left'},
        {to: '/blog', label: '개발일지', position: 'left'},
      ],
    },
  },
};

export default config;
