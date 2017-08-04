/* eslint import/no-extraneous-dependencies: 0 */
import React from 'react';
import { storiesOf } from '@storybook/react';
import { action } from '@storybook/addon-actions';
import Input from './Input';

/* globals module */

storiesOf('Input.Input', module)
  .add('default', () => (<Input placeholder="Input" value="PREDEFined value" />))
  .add('no input', () => (<Input placeholder="Input" />))
  .add('unwide', () => (<Input placeholder="Input" wide={false} />))
  .add('disabled', () => (<Input placeholder="Input" value="value" disabled />))
  .add('clearable', () => (<Input placeholder="Input" value="value" clearable />))
  .add('loading', () => (<Input placeholder="Input" value="value" loading />))
  .add('with callback', () =>
    (<Input placeholder="Input" value="change me" onChange={action('input')} />));